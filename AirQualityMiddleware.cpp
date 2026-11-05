/*
 * Author Jacob A. Psimos
 */

#include "AirQualityMiddleware.h"
#include "HygrometerMiddleware.h"
#include "SdMiddleware.h"

AirQualityMiddleware* AirQualityMiddleware::CreateInstance()
{
  if(AirQualityMiddleware::sm_singletonPointer == nullptr)
  {
    AirQualityMiddleware::sm_singletonPointer = new AirQualityMiddleware();
  }
  return AirQualityMiddleware::sm_singletonPointer;
}

AirQualityMiddleware* AirQualityMiddleware::GetInstance()
{
  return AirQualityMiddleware::sm_singletonPointer;
}

AirQualityMiddleware::AirQualityMiddleware() :
  Middleware(Middleware::AbstractType::CCS811_SENSOR),
  m_ccs811(new Adafruit_CCS811{}),
  m_calibrating(false),
  m_calibrated(false),
  m_updatedBaseline(false),
  m_failedToCalibrate(false),
  m_failedToUpdateBaseline(false),
  m_failedToUpdateEnvironment(false),
  m_calibrateTimer(),
  m_updateBaselineTimer(),
  m_updateEnvironmentTimer()
{
  if(m_ccs811 != nullptr)
  {
    m_ccs811->begin();
    m_calibrateTimer = 0;
    m_updateBaselineTimer = 0;
    m_updateEnvironmentTimer = 0;
  }
}

AirQualityMiddleware::~AirQualityMiddleware()
{
  if(m_ccs811)
  {
    delete m_ccs811;
  }
}

// Calibrate is a software state in this sketch such that the sensor gets its baseline one of two ways:
// 1. Feather M0 powers up and loads the previously saved baseline after CCS811_WARM_UP_TIME.
// 2. Serial/API command is received before CCS811_WARM_UP_TIME.
// If option 2 occurs, save off the baseline for when option 1 is needed.
bool AirQualityMiddleware::Calibrate()
{
  bool hadError;
  uint16_t baseline;
  
  hadError = m_calibrating || m_calibrated || m_updatedBaseline 
    || SdMiddleware::GetInstance() == nullptr  || HygrometerMiddleware::GetInstance() == nullptr;
  
  if(!hadError)
  {
    m_calibrating = true;
  }

  return !hadError;
}

bool AirQualityMiddleware::UpdateBaseline()
{
  bool hadError;
  uint16_t baseline;

  hadError = !(SdMiddleware::GetInstance() != nullptr && HygrometerMiddleware::GetInstance() != nullptr);

  if(!hadError)
  {
    hadError = !SdMiddleware::GetInstance()->LoadAirQualityBaseline(baseline);
    
    if(!hadError)
    {
      m_ccs811->setBaseline(baseline);
    }
  }
  
  return !hadError;
}

bool AirQualityMiddleware::UpdateEnvironment()
{
  bool hadError;
  float temperature;
  float humidity;

  hadError = HygrometerMiddleware::GetInstance() == nullptr;

  if(!hadError)
  {
    hadError = !HygrometerMiddleware::GetInstance()->Sample(&temperature, &humidity, false);
    
    if(!hadError)
    {
      m_ccs811->setEnvironmentalData(humidity, temperature);
    }
  }

  return !hadError;
}

void AirQualityMiddleware::Loop()
{
  bool hadError;
  float temperature;
  float humidity;
  uint16_t baseline;

  if(m_ccs811 != nullptr)
  {
    // Calibration procedure occurs during sensor powerup and CCS811_CALIBRATION_TIME.
    // Once m_updatedBaseline is true, it is too late to call Calibrate().
    // If saving the baseline fails after CCS811_CALIBRATION_TIME, do not keep trying.
    if(m_calibrating && !m_failedToCalibrate && m_calibrateTimer >= CCS811_CALIBRATION_TIME)
    {
      baseline = m_ccs811->getBaseline();
      hadError = !SdMiddleware::GetInstance()->SaveAirQualityBaseline(baseline);

      if(!hadError)
      {
        m_calibrating = false;
        m_calibrated = true;
      }
    }
    
    // Try to update the baseline once after CCS811_WARM_UP_TIME, unless calibrating.
    if(!m_updatedBaseline && !m_calibrating && !m_calibrated)
    {
      if(!m_failedToUpdateBaseline && m_updateBaselineTimer >= CCS811_WARM_UP_TIME)
      {
        hadError = !UpdateBaseline();
        
        if(!hadError)
        {
          m_updatedBaseline = true;
          m_calibrated = true;
        }
        else
        {
          m_failedToUpdateBaseline = true;
        }
      }
    }
  
    // Periodically update the environment data. If it fails, do not keep trying.
    if(!m_failedToUpdateEnvironment && m_updateEnvironmentTimer >= CCS811_UPDATE_ENVIRONMENT_TIME && m_ccs811->available())
    {
      hadError = !UpdateEnvironment();
      m_failedToUpdateEnvironment = hadError;
    }
  }
}

int AirQualityMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;
  int numBytesRead;
  int baseline;
  float temperature;
  float humidity;

  if(m_ccs811 != nullptr && inputLineLength > 3 && inputLine[0] == 'a' && inputLine[1] == 'q' && inputLine[2] == ' ')
  {
    if(Text::CStrEq(inputLine, "aq status"))
    {
      Serial.print("Calibrated: ");
      Serial.println(m_calibrated ? "true" : "false");
      Serial.print("Calibrating: ");
      Serial.println(m_calibrating ? "true" : "false");
      Serial.printf("Baseline: %u\r\n", m_ccs811->getBaseline());
      commandResult = COMMAND_OK;
    }
    else if(Text::CStrEq(inputLine, "aq read"))
    {
      if(m_ccs811->available())
      {
        if(!m_ccs811->readData())
        {
          Serial.print("CO2: ");
          Serial.print(m_ccs811->geteCO2());
          Serial.print(", TVOC: ");
          Serial.println(m_ccs811->getTVOC());
          commandResult = COMMAND_OK;
        }
        else
        {
          commandResult = COMMAND_ERROR;
        }
      }
      else
      {
        commandResult = COMMAND_ERROR;
      }
    }
    else if(Text::CStrEq(inputLine, "aq cal"))
    {
      commandResult = Calibrate() ? COMMAND_OK : COMMAND_ERROR;
    }
  }

  return commandResult;
}

int AirQualityMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;

  if(m_ccs811 != nullptr && inputLineLength > 3 && inputLine[0] == 'a' && inputLine[1] == 'q' && inputLine[2] == ' ')
  {
    if(Text::CStrEq(inputLine, "aq status"))
    {
      client.print("{\"calibrated\":");
      client.print(m_calibrated ? "true" : "false");
      client.print("{\"calibrating\":");
      client.print(m_calibrating ? "true" : "false");
      client.println("}");
      commandResult = COMMAND_OK;
    }
    else if(Text::CStrEq(inputLine, "aq read"))
    {
      if(m_ccs811->available())
      {
        if(!m_ccs811->readData())
        {
          client.print("{\"co2\": ");
          client.print(m_ccs811->geteCO2());
          client.print(", \"tvoc\": ");
          client.print(m_ccs811->getTVOC());
          client.println("}");
          
          commandResult = COMMAND_OK;
        }
        else
        {
          commandResult = COMMAND_ERROR;
        }
      }
      else
      {
        commandResult = COMMAND_ERROR;
      }
    }
    else if(Text::CStrEq(inputLine, "aq cal"))
    {
      commandResult = Calibrate() ? COMMAND_OK : COMMAND_ERROR;
    }
  }

  return commandResult;
}
