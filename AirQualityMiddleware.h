/*
 * Author Jacob A. Psimos
 */

 #ifndef __AIR_QUALITY_MW_H
 #define __AIR_QUALITY_MW_H

#include "Common.h"
#include "Middleware.h"
#include <Arduino.h>
#include <elapsedMillis.h>
#include <Adafruit_CCS811.h>

class AirQualityMiddleware : public Middleware
{
  private:
    inline static AirQualityMiddleware* sm_singletonPointer = nullptr;

  public:
    static AirQualityMiddleware* CreateInstance();
    static AirQualityMiddleware* GetInstance();
    bool Configure();
    
  private:
    AirQualityMiddleware();
    virtual ~AirQualityMiddleware();
    virtual void Loop();
    virtual int ProcessSerialCommand(const char inputLine[], size_t inputLineLength);
    virtual int ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client);

  private:
    Adafruit_CCS811* m_ccs811;
    bool m_configured;
};


 #endif
