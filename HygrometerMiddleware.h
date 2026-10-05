/*
 * Author Jacob A. Psimos
 */

#ifndef __HYGMIDDLEWARE_H
#define __HYGMIDDLEWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <elapsedMillis.h>

class HygrometerMiddleware : public Middleware
{
  private:
    inline static HygrometerMiddleware* sm_singletonPointer = nullptr;
    
  private:
    HygrometerMiddleware();
    virtual ~HygrometerMiddleware() = default;

  public:
    static HygrometerMiddleware* CreateInstance();
    static HygrometerMiddleware* GetInstance();
    virtual bool Initialize();
    bool Sample(float* temperature, float* humidity);
    float GetMaxTemperature();
    float GetMinTemperature();
    float GetMaxHumidity();
    float GetMinHumidity();
    virtual void Loop();
    virtual int ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream);

  private:
    Adafruit_SHT4x* m_sht41;
    elapsedMillis m_hygSampleTimer;
    float m_temperature;
    float m_humidity;
    float m_maxTemperature;
    float m_maxHumidity;
    float m_minTemperature;
    float m_minHumidity;
};

#endif
