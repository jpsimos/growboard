/*
 * author Jacob A Psimos
 */

#include "WifiMiddleware.h"
#include "RtcMiddleware.h"
#include "SdMiddleware.h"
#include "SdMiddleware.h"
#include "HygrometerMiddleware.h"
#include "Common.h"

WifiMiddleware* WifiMiddleware::CreateInstance()
{
  if(nullptr == WifiMiddleware::sm_singletonPointer)
  {
    WifiMiddleware::sm_singletonPointer = new WifiMiddleware();
  }
  return WifiMiddleware::sm_singletonPointer;
}

WifiMiddleware* WifiMiddleware::GetInstance()
{
  return WifiMiddleware::sm_singletonPointer;
}

WifiMiddleware::WifiMiddleware() :
  Middleware(Middleware::AbstractType::WIFI_CARD),
  m_wifiClass(&WiFi),
  m_wifiServer(new WiFiServer{80}),
  m_loadedConfigFile(false),
  m_initialConnect(false),
  m_autoConnect(false),
  m_autoReconnect(false),
  m_verbose(true),
  m_clientTimer(),
  m_connectTimer(),
  m_ssid(),
  m_key()
{
  m_wifiClass->setPins(WF_CS_PIN, WF_IRQ_PIN, WF_RST_PIN, WF_EN_PIN);

  if(nullptr != SdMiddleware::GetInstance())
  {
    m_loadedConfigFile = SdMiddleware::GetInstance()->LoadConfigTxt(m_ssid, SSID_BUFFER_SIZE, m_key, KEY_BUFFER_SIZE, m_autoConnect, m_autoReconnect);
  }
}

WifiMiddleware::~WifiMiddleware()
{
  if(m_wifiServer != nullptr)
  {
    m_wifiServer->stop();
    delete m_wifiServer;
  }
}

bool WifiMiddleware::Connected() const
{
  if(m_wifiClass != nullptr)
  {
    return static_cast<bool>(m_wifiClass->status() == WL_CONNECTED);
  }
  return false;
}

void WifiMiddleware::Loop()
{
  // Load the Config.txt file if it isn't already and connect to WiFi. Retry timer is 10 seconds.
  // Then process the HTTP Server if the WiFi is connected.
  
  if(!m_loadedConfigFile)
  {
    if(nullptr != SdMiddleware::GetInstance())
    {
      m_loadedConfigFile = SdMiddleware::GetInstance()->LoadConfigTxt(m_ssid, SSID_BUFFER_SIZE, m_key, KEY_BUFFER_SIZE, m_autoConnect, m_autoReconnect);
    }
  }
  else if(!m_initialConnect && m_autoConnect && !Connected())
  {
    if(m_connectTimer > 10000)
    {
      m_wifiClass->begin(m_ssid, m_key);
      m_initialConnect = true;
      m_connectTimer = 0;
    }
  }
  else if(m_initialConnect && m_autoReconnect && !Connected())
  {
    if(m_connectTimer > 10000)
    {
      m_wifiClass->begin(m_ssid, m_key);
      m_connectTimer = 0;
    }
  }
  else if(m_initialConnect & !Connected() && m_wifiServer->status())
  {
    m_wifiServer->stop();
  }
  else if(m_initialConnect && Connected() && !m_wifiServer->status())
  {
    m_wifiServer->begin();
  }
  
  if(Connected() && m_wifiServer->status())
  {
    ProcessHttpServer();
  }
}

int WifiMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;
  unsigned int led;
  unsigned int value;

  if(Text::CStrEq(inputLine, "wifi scan"))
  {
      int numSsid = m_wifiClass->scanNetworks(true);
      
      for(int thisNet = 0; thisNet < numSsid; thisNet++)
      {
        uint8_t macAddr[6];
        m_wifiClass->BSSID(thisNet, macAddr);
        Serial.printf("%2u ", m_wifiClass->channel(thisNet));
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x ", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
        Serial.print(m_wifiClass->RSSI(thisNet));
        Serial.print(' ');
        Serial.println(m_wifiClass->SSID(thisNet));
      }
      commandResult |= COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "wifi status"))
  {
    const int wifiStatus = (int)m_wifiClass->status();
    Serial.print("Status: ");
    Serial.println(wifiStatus);
    Serial.print("AutoReconnect: ");
    Serial.println(m_autoReconnect ? "true " : "false ");
    Serial.print("Connected: ");
    Serial.println(Connected() ? "true" : "false");
    
    if(Connected())
    {
      Serial.print("SSID: ");
      Serial.println(m_wifiClass->SSID());
      Serial.printf("RSSI: %d\n", (int)(m_wifiClass->RSSI()));
      Serial.print("IP Address: ");
      Serial.println(IPAddress(m_wifiClass->localIP()));
      Serial.print("http: ");
      Serial.printf("(status = %d)\r\n", (int)m_wifiServer->status());
    }
    commandResult |= COMMAND_OK;
  }

  return commandResult;
}

void WifiMiddleware::ProcessHttpServer()
{
  bool timedOut = false;
  ssize_t requestLength = 0;
  size_t requestParamsLength = 0;
  size_t requestParamsCount = 0;
  char requestBuffer[REQUEST_BUFFER_SIZE];
  char requestParamNames[MAX_REQUEST_PARAMS][REQUEST_PARAM_NAME_BUFFER_SIZE];
  char requestParamValues[MAX_REQUEST_PARAMS][REQUEST_PARAM_VALUE_BUFFER_SIZE];
  char* responseBuffer = requestBuffer;

  // Every 10ms, handle client connections and process their HTTP requests.
  // Parse the GET parameter(s,) if any, and send "text/html" or "text/json"
  // based on the presense of GET param(s.)
  
  if(m_clientTimer >= 10)
  {
    WiFiClient client = m_wifiServer->available();
    
    if(client)
    {
      for(elapsedMillis timeout; !timedOut; timedOut = (bool)(timeout >= 200UL))
      {
        if(client.connected() && requestLength < (REQUEST_BUFFER_SIZE - 1))
        {
          if(client.available())
          {
            requestBuffer[requestLength++] = static_cast<char>(client.read());
            requestBuffer[requestLength] = '\0';
            if(requestLength > 3)
            {
              if(Text::CStrEq(&requestBuffer[requestLength - 3], "\n\r\n"))
              {
                break;
              }
            }
            timeout = 0;
          }
        }
        else
        {
          requestLength = 0;
          break;
        }
      }
      
      if(!timedOut && requestLength > 0)
      {
        if(m_verbose)
        {
          Serial.println(requestBuffer);
        }

        client.print(
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/"
        );
        
        if(requestLength > 4 && requestBuffer == strstr(requestBuffer, "GET"))
        {
          char* requestParams = strchr(requestBuffer, '?');
          char* requestParamsEnd = requestParams ? strstr(++requestParams, " HTTP/1.1\r\n") : nullptr;
          requestParamsLength = (size_t)max(0, (ssize_t)requestParamsEnd - (ssize_t)requestParams);
          requestParamsCount = ParseRequestParams(requestParams, requestParamsLength, requestParamNames, requestParamValues);
        
          if(!requestParamsCount)
          {
            client.println("html");
          }
          else
          {
            client.println("json");
          }

          client.println(
            "Connection: close\r\n"
            "\r\n"
          );

          for(size_t param = 0; param < requestParamsCount; param++)
          {
            const char* const paramName = requestParamNames[param];
            const int paramNameLength = strlen(paramName);
            const char* const paramValue = requestParamValues[param];
            const int paramValueLength = strlen(paramValue);
            
            if(paramNameLength)
            {
              if(m_verbose)
              {
                Serial.print(paramName);
                Serial.print(" = ");
                if(paramValueLength)
                {
                  Serial.println(paramValue);
                }
              }
              if(paramValueLength && Text::CStrEq(paramName, "command"))
              {
                for(size_t middlewareIndex = 0; middlewareIndex < Middleware::NumMiddlewares; middlewareIndex++)
                {
                  Middleware* middlewarePointer = Middleware::GetMiddlewares()[middlewareIndex];
                  
                  if(nullptr != middlewarePointer)
                  {
                    middlewarePointer->ProcessRestfulApiCommand(paramValue, paramValueLength, client);
                  }
                }
              }
            }
          }
        }
        
        if(!requestParamsCount && nullptr != SdMiddleware::GetInstance())
        {
          SdMiddleware::GetInstance()->ReadFile(
            "/Index.html",
            (unsigned char*)requestBuffer,
            min(64, REQUEST_BUFFER_SIZE),
            [&](size_t chunkLength, unsigned char chunk[]) -> bool{
              client.write(chunk, chunkLength);
              return true;
            }
          );
        }
        
        client.stop();
      }
    }
    
    m_clientTimer = 0;
  }
}

/*
 * Parse GET parameter(s) from the request.
 */
size_t WifiMiddleware::ParseRequestParams(
  char request[],
  const size_t requestLength,
  char requestParamNames[][REQUEST_PARAM_NAME_BUFFER_SIZE], 
  char requestParamValues[][REQUEST_PARAM_VALUE_BUFFER_SIZE]
)
{
  size_t numParams = 0;
  size_t paramNameLength = 0;
  size_t paramValueLength = 0;
  size_t paramValueDecodedLength = 0;
  char* saveptr1 = nullptr;
  char* saveptr2 = nullptr;
  char* str1 = nullptr;
  char* str2 = nullptr;
  char* token = nullptr;
  char* subtoken = nullptr;
  char* paramName = nullptr;
  char* paramValue = nullptr;
  bool bail = false;
  const char term = request[requestLength];
  char paramValueDecoded[REQUEST_PARAM_VALUE_BUFFER_SIZE];
  char encodedByteStr[3];
  
  if(requestLength > 0)
  {
    request[requestLength] = '\0';
    for(str1 = request; ; str1 = nullptr)
    {
      token = strtok_r(str1, "&", &saveptr1);
      if(token == nullptr)
      {
        break;
      }
      for(str2 = token; ; str2 = nullptr)
      {
        subtoken = strtok_r(str2, "=", &saveptr2);
        if(subtoken == nullptr)
        {
          break;
        }
        if(paramName == nullptr)
        {
          paramName = subtoken;
          paramNameLength = min(REQUEST_PARAM_NAME_BUFFER_SIZE - 1, strlen(paramName));
        }
        else if(paramValue == nullptr)
        {
          paramValue = subtoken;
          paramValueLength = min(REQUEST_PARAM_VALUE_BUFFER_SIZE - 1, strlen(paramValue));

          if(numParams < MAX_REQUEST_PARAMS && paramNameLength > 0 && paramValueLength > 0)
          {
            strncpy(requestParamNames[numParams], paramName, REQUEST_PARAM_NAME_BUFFER_SIZE);
            requestParamNames[numParams][paramNameLength] = '\0';
            
            for(size_t i = 0; i < paramValueLength; i += 1)
            {
              if(paramValue[i] == '%' && paramValueLength > (i + 2))
              {
                encodedByteStr[0] = paramValue[++i];
                encodedByteStr[1] = paramValue[++i];
                encodedByteStr[2] = '\0';
                paramValueDecoded[paramValueDecodedLength++] = static_cast<char>(strtol(encodedByteStr, nullptr, 16));
              }
              else
              {
                paramValueDecoded[paramValueDecodedLength++] = paramValue[i]; 
              }
            }
            paramValueDecoded[paramValueDecodedLength] = '\0';
            strncpy(requestParamValues[numParams], paramValueDecoded, REQUEST_PARAM_VALUE_BUFFER_SIZE);
            
            numParams += 1;
          }
          
          paramName = nullptr;
          paramValue = nullptr;
          paramValueDecodedLength = 0;
        }
      }
    }
    request[requestLength] = term;
  }
  
  return numParams;
}
