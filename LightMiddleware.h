

/*
 * Author Jacob A. Psimos
 */

 #ifndef __LIGHT_MIDDLEWARE_H
 #define __LIGHT_MIDDLEWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <Wire.h>
#include <elapsedMillis.h>
#include "Common.h"

#define LIGHT_SENSOR_MW_RATE 1000

class LightMiddleware : public Middleware
{
  private:
    inline static LightMiddleware* sm_singletonPointer = nullptr;
    
  public:
    static LightMiddleware* CreateInstance();
    static LightMiddleware* GetInstance();

  private:
    LightMiddleware();
    virtual ~LightMiddleware() = default;

  public:
    bool Read(float* pLux, float* pTemperature, float* pPressure, float* pHumidity, float* pAltitude); 
    virtual void Loop();
    virtual int ProcessSerialCommand(const char inputLine[], size_t inputLineLength);
    virtual int ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client);
};

 #endif
