/*
 * author Jacob A Psimos
 */

#ifndef __REALTIMECLOCK_MIDDLWARE_H
#define __REALTIMECLOCK_MIDDLWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <RTClib.h>
#include <elapsedMillis.h>
#include <functional>

// Pin definitions
#define RTC1_SQWPIN 6

class RtcMiddleware : public Middleware
{
  private:
    inline static RtcMiddleware* sm_singletonPointer = nullptr;
    
  public:
    static RtcMiddleware* CreateInstance();
    static RtcMiddleware* GetInstance();
    virtual bool Initialize();
    DateTime Now();
    const char* const NowString();
    virtual int ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream);

  private:
    RtcMiddleware(const bool interruptEnable = false);
    virtual ~RtcMiddleware();
    static void HandleIsr();
    
  private:
    const bool m_interruptEnable;
    RTC_PCF8523* m_rtcBoard;
};

#endif
