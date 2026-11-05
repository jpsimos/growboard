/*
 * Author Jacob A. Psimos
 */

#include "LightMiddleware.h"

LightMiddleware* LightMiddleware::CreateInstance()
{
  if(nullptr == LightMiddleware::sm_singletonPointer)
  {
    LightMiddleware::sm_singletonPointer = new LightMiddleware();
  }
  return LightMiddleware::sm_singletonPointer;
}

LightMiddleware* LightMiddleware::GetInstance()
{
  return LightMiddleware::sm_singletonPointer;
}

LightMiddleware::LightMiddleware()
  : Middleware(Middleware::AbstractType::LIGHT_SENSOR)
{
}

bool LightMiddleware::Read(float* pLux, float* pTemperature, float* pPressure, float* pHumidity, float* pAltitude)
{
  bool hadError;
  uint8_t buf[14];
  float lux;
  float temp;
  float pressure;
  float hum;
  float altitude;
  uint32_t p;
  uint16_t alt;
  
  Wire.beginTransmission(0x5B);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(0x5B, 14);

  for(unsigned int i = 0; i < 14; i++)
  {
    buf[i] = Wire.read();
  }

  lux = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
    ((uint32_t)buf[2] << 8)  | buf[3];
    
  temp  = ((buf[4] << 8) | buf[5]) / 100.0f;
  temp = ((temp * 9.0f) / 5.0f) + 32.0f;
  
  p  = ((uint32_t)buf[6] << 24) | ((uint32_t)buf[7] << 16) |
    ((uint32_t)buf[8] << 8)  | buf[9];
    
  pressure = (float)p / 100.0f;

  hum   = ((buf[10] << 8) | buf[11]) / 100.0f;
  
  alt = (buf[12] << 8) | buf[13];
  altitude = (float)alt * 3.28084f;

  if(pLux != nullptr)
    *pLux = lux;

  if(pTemperature != nullptr)
    *pTemperature = temp;

  if(pPressure != nullptr)
    *pPressure = pressure;

  if(pHumidity != nullptr)
    *pHumidity = hum;

  if(pAltitude != nullptr)
    *pAltitude = altitude;
  
  return !hadError;
}

void LightMiddleware::Loop()
{
  return;
}

int LightMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;
  float lux;
  float temperature;
  float pressure;
  float humidity;
  float altitude;
  
  if(Text::CStrEq(inputLine, "light read"))
  {
    if(Read(&lux, &temperature, &pressure, &humidity, &altitude))
    {
      client.print("{\"lux\":");
      client.print(lux);
      client.print(", \"temperature\":");
      client.print(temperature);
      client.print(", \"pressure\":");
      client.print(pressure);
      client.print(", \"humidity\":");
      client.print(humidity);
      client.print(", \"altitude\":");
      client.print(altitude);
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

int LightMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;
  float lux;
  float temperature;
  float pressure;
  float humidity;
  float altitude;
  
  if(Text::CStrEq(inputLine, "light read"))
  {
    if(Read(&lux, &temperature, &pressure, &humidity, &altitude))
    {
      Serial.print("Lux: ");
      Serial.println(lux);
      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Pressure: ");
      Serial.println(pressure);
      Serial.print("Humidity: ");
      Serial.println(humidity);
      Serial.print("Altitude: ");
      Serial.println(altitude);
      
      commandResult = COMMAND_OK;
    }
    else
    {
      commandResult = COMMAND_ERROR;
    }
  }
  return commandResult;
}
