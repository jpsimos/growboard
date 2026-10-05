/*
 * author Jacob A Psimos
 */

#ifndef __WIFI_MIDDLEWARE_H
#define __WIFI_MIDDLEWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <WiFi101.h>
#include <elapsedMillis.h>

// Pin definitions
#define WF_CS_PIN 8
#define WF_IRQ_PIN 7
#define WF_RST_PIN 4
#define WF_EN_PIN 2

// Other definitions
#define REQUEST_BUFFER_SIZE 1024
#define RESPONSE_BUFFER_SIZE REQUEST_BUFFER_SIZE
#define MAX_REQUEST_PARAMS 2
#define REQUEST_PARAM_NAME_BUFFER_SIZE 24
#define REQUEST_PARAM_VALUE_BUFFER_SIZE 256

class WifiMiddleware : public Middleware
{
  private:
    inline static WifiMiddleware* sm_singletonPointer = nullptr;
  
  public:
    static WifiMiddleware* CreateInstance();
    static WifiMiddleware* GetInstance();
    virtual bool Initialize();
    bool Connected() const;
    void SetAutoconnect(const bool autoConnect);
    void SetHttpEnabled(const bool httpEnable);
    virtual void Loop();
    virtual int ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream);
    
  private:
    WifiMiddleware();
    virtual ~WifiMiddleware();
    void ProcessHttpServer();
    size_t ParseRequestParams(char request[], const size_t requestLength,
      char requestParamNames[][REQUEST_PARAM_NAME_BUFFER_SIZE], char requestParamValues[][REQUEST_PARAM_VALUE_BUFFER_SIZE]);

  private:
    WiFiClass* m_wifiClass;
    WiFiServer* m_wifiServer;
    bool m_autoConnect;
    bool m_enableHttp;
    bool m_verbose;
    elapsedMillis m_clientTimer;
};

#endif
