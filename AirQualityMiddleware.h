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

// Definitions
#define CCS811_WARM_UP_TIME             (20 * 1000 * 60)        // 20 Minutes
#define CCS811_CALIBRATION_TIME         (60 * 1000 * 60 * 24)   // 24 Hours
#define CCS811_UPDATE_ENVIRONMENT_TIME  (1000)                  // CCS811_DRIVE_MODE_1SEC

class AirQualityMiddleware : public Middleware
{
  private:
    inline static AirQualityMiddleware* sm_singletonPointer = nullptr;

  public:
    static AirQualityMiddleware* CreateInstance();
    static AirQualityMiddleware* GetInstance();
    
  private:
    AirQualityMiddleware();
    bool UpdateBaseline();
    bool UpdateEnvironment();
    virtual ~AirQualityMiddleware();
    bool Calibrate();
    virtual void Loop();
    virtual int ProcessSerialCommand(const char inputLine[], size_t inputLineLength);
    virtual int ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client);

  private:
    Adafruit_CCS811* m_ccs811;
    bool m_calibrating;
    bool m_calibrated;
    bool m_updatedBaseline;
    bool m_failedToCalibrate;
    bool m_failedToUpdateBaseline;
    bool m_failedToUpdateEnvironment;
    elapsedMillis m_calibrateTimer;
    elapsedMillis m_updateBaselineTimer;
    elapsedMillis m_updateEnvironmentTimer;
};


 #endif
