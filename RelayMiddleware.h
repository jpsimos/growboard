/*
 * author Jacob A Psimos
 */

#ifndef __RELAY_MIDDLWARE_H
#define __RELAY_MIDDLWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <elapsedMillis.h>

// Pin Definitions
#define RELAY_PIN 5

// Other definitions
#define RELAY_DEBOUNCE_TIME   100
#define RELAY_CMD_NONE   0
#define RELAY_CMD_ON     1
#define RELAY_CMD_OFF    2

class RelayMiddleware : public Middleware
{
  private:
    inline static RelayMiddleware* sm_singletonPointer = nullptr;
    
  public:
    static RelayMiddleware* CreateInstance();
    static RelayMiddleware* GetInstance();
    void Command(const int command);
    virtual void Loop();
    virtual int ProcessSerialCommand(const char inputLine[], size_t inputLineLength);
    virtual int ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client);
    
  private:
    RelayMiddleware();
    virtual ~RelayMiddleware();
    
  private:
    elapsedMillis m_debounceTimer;
    int m_command;
};

#endif
