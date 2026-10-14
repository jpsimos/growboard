/*
 * Author Jacob A. Psimos
 */

#include "HygrometerMiddleware.h"
#include "Common.h"
#include <limits>

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
  m_hygSampleTimer(0U),
  m_temperature(0.0f),
  m_humidity(0.0f),
  m_maxTemperature(std::numeric_limits<float>::min()),
  m_maxHumidity(std::numeric_limits<float>::min()),
  m_minTemperature(std::numeric_limits<float>::max()),
  m_minHumidity(std::numeric_limits<float>::max())
{
    if(m_sht41->begin())
    {
      m_sht41->setPrecision(SHT4X_MED_PRECISION);
      m_sht41->setHeater(SHT4X_MED_HEATER_100MS);
    }
}

bool HygrometerMiddleware::Sample(float* temperature, float* humidity, const bool fahrenheit)
{
  sensors_event_t humidity_;
  sensors_event_t temperature_;
  
  if(m_sht41 != nullptr)
  {
    if(m_hygSampleTimer > 200)
    {
      m_hygSampleTimer = 0;
      if(m_sht41->getEvent(&humidity_, &temperature_))
      {
        m_temperature = fahrenheit ? ((((float)temperature_.temperature * 9.0) / 5.0) + 32.0) : (float)temperature_.temperature;
        m_humidity = (float)humidity_.relative_humidity;
        if(m_temperature > m_maxTemperature)
        {
          m_maxTemperature = m_temperature;
        }
        else if(m_temperature < m_minTemperature)
        {
          m_minTemperature = m_temperature;
        }
        if(m_humidity > m_maxHumidity)
        {
          m_maxHumidity = m_humidity;
        }
        else if(m_humidity < m_minHumidity)
        {
          m_minHumidity = m_humidity;
        }
      }
      else
      {
        return false;
      }
    }
    if(temperature != nullptr)
    {
      *temperature = m_temperature;
    }
    if(humidity != nullptr)
    {
      *humidity = m_humidity;
    }
    return true;
  }
  return false;
}

float HygrometerMiddleware::GetMaxTemperature()
{
  return m_maxTemperature;
}

float HygrometerMiddleware::GetMinTemperature()
{
  return m_minTemperature;
}

float HygrometerMiddleware::GetMaxHumidity()
{
  return m_maxHumidity;
}

float HygrometerMiddleware::GetMinHumidity()
{
  return m_minHumidity;
}

int HygrometerMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;

  if(Text::CStrEq(inputLine, "hyg status"))
  {
    if(Sample(nullptr, nullptr))
    {
      client.print("{\"temperature\":");
      client.print(m_temperature);
      client.print(",\"humidity\":");
      client.print(m_humidity);
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
  if(Text::CStrEq(inputLine, "hyg read"))
  {
    if(Sample(nullptr, nullptr))
    {
      Serial.print("Temperature: ");
      Serial.print(m_temperature);
      Serial.println(" F");
      Serial.print("Humidity: ");
      Serial.print(m_humidity);
      Serial.println("% rH");
      commandResult = COMMAND_OK;
    }
    else
    {
      commandResult = COMMAND_ERROR;
    }
  }
  else if(Text::CStrEq(inputLine, "hyg status"))
  {
    if(Sample(nullptr, nullptr))
    {
      Serial.print("Temperature: ");
      Serial.print(m_temperature);
      Serial.print(", ");
      Serial.print(m_minTemperature);
      Serial.print(", ");
      Serial.print(m_maxTemperature);
      Serial.println(" F");
      Serial.print("Humidity: ");
      Serial.print(m_humidity);
      Serial.print(", ");
      Serial.print(m_minHumidity);
      Serial.print(", ");
      Serial.print(m_maxHumidity);
      Serial.println(" %");
      commandResult = COMMAND_OK;
    }
    else
    {
      commandResult = COMMAND_ERROR;
    }
  }
  return commandResult;
}
