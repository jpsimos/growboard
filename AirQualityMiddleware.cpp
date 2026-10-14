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
  m_configured(false)
{
  if(m_ccs811 != nullptr)
  {
    m_ccs811->begin();
  }
}

AirQualityMiddleware::~AirQualityMiddleware()
{
  if(m_ccs811)
  {
    delete m_ccs811;
  }
}

bool AirQualityMiddleware::Configure()
{
  bool hadError;
  uint16_t baseline;
  float temperature;
  float humidity;
  
  hadError = !(m_ccs811 != nullptr && SdMiddleware::GetInstance() != nullptr && HygrometerMiddleware::GetInstance() != nullptr);
  
  if(!hadError)
  {
    hadError = !SdMiddleware::GetInstance()->LoadAirQualityBaseline(baseline);

    if(!hadError)
    {
      m_ccs811->setBaseline(baseline);
    }
    else
    {
      baseline = m_ccs811->getBaseline();
      hadError = !SdMiddleware::GetInstance()->SaveAirQualityBaseline(baseline);
    }

    if(!hadError)
    {
      hadError = !HygrometerMiddleware::GetInstance()->Sample(&temperature, &humidity, false);
      
      if(!hadError)
      {
        m_ccs811->setEnvironmentalData(humidity, temperature);
      }
    }
  }

  return !hadError;
}

void AirQualityMiddleware::Loop()
{
  if(!m_configured)
  {
    m_configured = Configure();
  }
}

int AirQualityMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;
  int numBytesRead;
  int baseline;
  float temperature;
  float humidity;

  if(Text::CStrEq(inputLine, "aq status"))
  {
    Serial.print("Configured: ");
    Serial.println(m_configured ? "true" : "false");
    Serial.printf("Baseline: %u\r\n", m_ccs811->getBaseline());
    commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "aq read"))
  {
    if(m_ccs811->available())
    {
      if(m_ccs811->readData())
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

  return commandResult;
}

int AirQualityMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;

  if(Text::CStrEq(inputLine, "aq status"))
  {
    client.print("{\"configured\":");
    client.print(m_configured ? "true" : "false");
    client.println("}");
    commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "aq read"))
  {
    if(m_ccs811->available())
    {
      if(m_ccs811->readData())
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

  return commandResult;
}
