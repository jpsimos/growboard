/*
 * Author Jacob A. Psimos
 */

#include "HygrometerMiddleware.h"
#include "Common.h"

HygrometerMiddleware* HygrometerMiddleware::CreateInstance()
{
  if(nullptr == HygrometerMiddleware::sm_singletonPointer)
  {
    HygrometerMiddleware::sm_singletonPointer = new HygrometerMiddleware();
  }
  return HygrometerMiddleware::sm_singletonPointer;
}

HygrometerMiddleware* HygrometerMiddleware::GetInstance()
{
  return HygrometerMiddleware::sm_singletonPointer;
}

HygrometerMiddleware::HygrometerMiddleware() :
  Middleware(Middleware::AbstractType::HYGROMETER_SENSOR),
  m_sht41(new Adafruit_SHT4x()),
  m_hygSampleTimer()
{
    if(m_sht41->begin())
    {
      m_sht41->setPrecision(SHT4X_MED_PRECISION);
      m_sht41->setHeater(SHT4X_MED_HEATER_100MS);
    }
}

bool HygrometerMiddleware::Sample(float& temperature, float& humidity, const bool fahrenheit)
{
  sensors_event_t humidity_;
  sensors_event_t temperature_;
  
  if(m_sht41 != nullptr && m_hygSampleTimer > 200)
  {
    m_hygSampleTimer = 0;
    
    if(m_sht41->getEvent(&humidity_, &temperature_))
    {
      temperature = fahrenheit ? ((((float)temperature_.temperature * 9.0) / 5.0) + 32.0) : (float)temperature_.temperature;
      humidity = (float)humidity_.relative_humidity;
      return true;
    }
  }
  
  return false;
}


int HygrometerMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;
  float temperature;
  float humidity;
  
  if(Text::CStrEq(inputLine, "hyg read"))
  {
    if(Sample(temperature, humidity))
    {
      client.print("{\"temperature\":");
      client.print(temperature);
      client.print(",\"humidity\":");
      client.print(humidity);
      client.println("}");
      commandResult = COMMAND_OK;
    }
    else
    {
      commandResult = COMMAND_ERROR;
    }
  }

  return commandResult;
}

int HygrometerMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;
  float temperature;
  float humidity;
  
  if(Text::CStrEq(inputLine, "hyg read"))
  {
    if(Sample(temperature, humidity, true))
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" F");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("% rH");
      commandResult = COMMAND_OK;
    }
    else
    {
      commandResult = COMMAND_ERROR;
    }
  }
  
  return commandResult;
}
