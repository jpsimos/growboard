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
  m_sht41(NULL),
  m_hygSampleTimer(0U),
  m_temperature(0.0f),
  m_humidity(0.0f),
  m_maxTemperature(std::numeric_limits<float>::min()),
  m_maxHumidity(std::numeric_limits<float>::min()),
  m_minTemperature(std::numeric_limits<float>::max()),
  m_minHumidity(std::numeric_limits<float>::max())
{
  Initialize();
}

bool HygrometerMiddleware::Initialize()
{
  if(!m_initialized)
  {
    if(m_sht41 == NULL)
    {
      m_sht41 = new Adafruit_SHT4x();
    }
    if(m_sht41 != NULL)
    {
      if(m_sht41->begin())
      {
        m_sht41->setPrecision(SHT4X_MED_PRECISION);
        m_sht41->setHeater(SHT4X_MED_HEATER_100MS);
        m_initialized = true;
      }
    }
  }
  return m_initialized;
}

bool HygrometerMiddleware::Sample(float* temperature, float* humidity)
{
  sensors_event_t humidity_;
  sensors_event_t temp;
  if(m_initialized)
  {
    if(m_hygSampleTimer > 200)
    {
      m_hygSampleTimer = 0;
      if(m_sht41->getEvent(&humidity_, &temp))
      {
        m_temperature = (((float)temp.temperature * 9.0) / 5.0) + 32.0;
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
    if(temperature != NULL)
    {
      *temperature = m_temperature;
    }
    if(humidity != NULL)
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

void HygrometerMiddleware::Loop()
{
}

int HygrometerMiddleware::ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream)
{
  int result = COMMAND_IGNORE;
  if(Text::streq(inputLine, "hyg sample"))
  {
    if(Sample(NULL, NULL))
    {
      stream->print("Temperature: ");
      stream->print(m_temperature);
      stream->println(" F");
      stream->print("Humidity: ");
      stream->print(m_humidity);
      stream->println("% rH");
      result = COMMAND_OK;
    }
    else
    {
      result = COMMAND_ERROR;
    }
  }
  else if(Text::streq(inputLine, "hyg stats"))
  {
    if(Sample(NULL, NULL))
    {
      stream->print("Temperature: ");
      stream->print(m_temperature);
      stream->print(", ");
      stream->print(m_minTemperature);
      stream->print(", ");
      stream->print(m_maxTemperature);
      stream->println(" F");
      stream->print("Humidity: ");
      stream->print(m_humidity);
      stream->print(", ");
      stream->print(m_minHumidity);
      stream->print(", ");
      stream->print(m_maxHumidity);
      stream->println(" %");
      result = COMMAND_OK;
    }
    else
    {
      result = COMMAND_ERROR;
    }
  }
  return result;
}
