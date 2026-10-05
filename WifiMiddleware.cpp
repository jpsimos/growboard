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
  m_autoConnect(false),
  m_enableHttp(false),
  m_verbose(true)
{
  m_wifiClass->setPins(WF_CS_PIN, WF_IRQ_PIN, WF_RST_PIN, WF_EN_PIN);
}

WifiMiddleware::~WifiMiddleware()
{
  if(m_wifiServer != NULL)
  {
    delete m_wifiServer;
  }
}

bool WifiMiddleware::Initialize()
{
  char ssid[33];
  char key[33];

  if(m_wifiServer)
  {
    if(!m_initialized)
    {
      if(SdMiddleware::GetInstance()->LoadConfigTxt(ssid, key, true) == 2)
      {
        if(m_autoConnect)
        {
          m_wifiClass->begin(ssid, key);
          m_autoConnect = false;
        }
        m_initialized = true;
      }
    }
    else
    {
      if(!Connected())
      {
        if(m_autoConnect && SdMiddleware::GetInstance()->LoadConfigTxt(ssid, key, false) == 2)
        {
          m_wifiClass->begin(ssid, key);
          m_autoConnect = false;
        }
        if(m_enableHttp || m_wifiServer->status())
        {
          m_wifiServer->stop();
        }
      }
      else
      {
        if(m_enableHttp && !m_wifiServer->status())
        {
          m_wifiServer->begin();
        }
        else if(!m_enableHttp && m_wifiServer->status())
        {
          m_wifiServer->stop();
        }
      }
    }
  }
  return m_initialized;
}

bool WifiMiddleware::Connected() const
{
  return static_cast<bool>(m_wifiClass->status() == WL_CONNECTED);
}

void WifiMiddleware::SetAutoconnect(const bool autoConnect)
{
  m_autoConnect = autoConnect;
}

void WifiMiddleware::SetHttpEnabled(const bool httpEnable)
{
  m_enableHttp = httpEnable;
}

void WifiMiddleware::Loop()
{
  if(!m_initialized)
  {
    Initialize();
  }
  else
  {
    if(!Connected())
    {
      if(m_autoConnect)
      {
        Initialize();
      }
    }
    else
    {
      if((m_enableHttp && !m_wifiServer->status()) || (!m_enableHttp && m_wifiServer->status()))
      {
        Initialize();
      }
      if(m_enableHttp && m_wifiServer->status())
      {
        ProcessHttpServer();
      }
    }
  }
}

int WifiMiddleware::ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream)
{
  int result = COMMAND_IGNORE;
  unsigned int led;
  unsigned int value;

  if(Text::streq(inputLine, "wifi scan"))
  {
      int numSsid = m_wifiClass->scanNetworks(true);
      for(int thisNet = 0; thisNet < numSsid; thisNet++)
      {
        uint8_t macAddr[6];
        m_wifiClass->BSSID(thisNet, macAddr);
        stream->printf("%2u ", m_wifiClass->channel(thisNet));
        stream->printf("%02x:%02x:%02x:%02x:%02x:%02x ", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
        stream->print(m_wifiClass->RSSI(thisNet));
        stream->print(' ');
        stream->println(m_wifiClass->SSID(thisNet));
      }
      result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "wifi status"))
  {
    const int wifiStatus = (int)m_wifiClass->status();
    stream->print("Initialized: ");
    stream->println(m_initialized ? "true" : "false");
    stream->print("Status: ");
    stream->println(wifiStatus);
    stream->print("AutoConnect: ");
    stream->print(m_autoConnect ? "true " : "false ");
    stream->println(Connected() ? "(done)" : "(await)");
    if(Connected())
    {
      stream->print("SSID: ");
      stream->println(m_wifiClass->SSID());
      stream->printf("RSSI: %d\n", (int)(m_wifiClass->RSSI()));
      stream->print("IP Address: ");
      stream->println(IPAddress(m_wifiClass->localIP()));
      stream->print("http: ");
      stream->print(m_enableHttp ? "enabled " : "disabled ");
      stream->printf("(status = %d)\r\n", (int)m_wifiServer->status());
    }
    result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "autoconnect enable"))
  {
    m_autoConnect = true;
    result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "autoconnect disable"))
  {
    m_autoConnect = false;
    result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "http disable"))
  {
    m_enableHttp = false;
    result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "http enable"))
  {
    m_enableHttp = true;
    result |= COMMAND_OK;
  }
  else if(Text::streq(inputLine, "http verbose"))
  {
    m_verbose = !m_verbose;
    result |= COMMAND_OK;
  }
  return result;
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
  
  if(m_clientTimer >= 10)
  {
    WiFiClient client = m_wifiServer->available();
    if (client)
    {
      for(elapsedMillis timeout; !timedOut; timedOut = (bool)(timeout >= 200UL))
      {
        if(client.connected() && requestLength < (REQUEST_BUFFER_SIZE - 1))
        {
          if(client.available())
          {
            const char nextChar = client.read();
            requestBuffer[requestLength++] = nextChar;
            requestBuffer[requestLength] = '\0';
            if(requestLength > 3)
            {
              if(Text::streq(&requestBuffer[requestLength - 3], "\n\r\n"))
              {
                break;
              }
            }
            timeout = 0UL;
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
          IO::InOutStream->println(requestBuffer);
        }

        client.print(
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Connection: close\r\n"
          "\r\n"
        );
        
        if(requestLength > 4 && requestBuffer == strstr(requestBuffer, "GET"))
        {
          char* requestParams = strchr(requestBuffer, '?');
          char* requestParamsEnd = requestParams ? strstr(++requestParams, " HTTP/1.1\r\n") : NULL;
          requestParamsLength = (size_t)max(0, (ssize_t)requestParamsEnd - (ssize_t)requestParams);
          requestParamsCount = ParseRequestParams(requestParams, requestParamsLength, requestParamNames, requestParamValues);

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
                IO::InOutStream->print(paramName);
                IO::InOutStream->print(" = ");
                if(paramValueLength)
                {
                  IO::InOutStream->println(paramValue);
                }
              }
              if(paramValueLength && Text::streq(paramName, "command"))
              {
                int commandResult = COMMAND_IGNORE;
                //commandResult |= RtcMiddleware::GetInstance()->ProcessSerialCommands(paramValue, paramValueLength, &client);
                //commandResult |= HygrometerMiddleware::GetInstance()->ProcessSerialCommands(paramValue, paramValueLength, &client);
                client.println((commandResult == COMMAND_OK) ? "OK" : "ERR");
              }
            }
          }
        }
        
        if(!requestParamsCount)
        {
          SdMiddleware::GetInstance()->ReadFile(
            "/Index.html",
            (unsigned char*)requestBuffer,
            64, 
            [&](size_t chunkLength, unsigned char chunk[]) -> bool{
              client.write(chunk, chunkLength);
              return true;
            }
          );
        }
        
        client.stop();
      }
    }
    m_clientTimer = 0UL;
  }
}

/*
 * Parse GET parameter(s) from the request.
 */
size_t WifiMiddleware::ParseRequestParams(char request[], const size_t requestLength,
  char requestParamNames[][REQUEST_PARAM_NAME_BUFFER_SIZE], char requestParamValues[][REQUEST_PARAM_VALUE_BUFFER_SIZE])
{
  size_t numParams = 0;
  size_t paramNameLength = 0;
  size_t paramValueLength = 0;
  size_t paramValueDecodedLength = 0;
  char* saveptr1 = NULL;
  char* saveptr2 = NULL;
  char* str1 = NULL;
  char* str2 = NULL;
  char* token = NULL;
  char* subtoken = NULL;
  char* paramName = NULL;
  char* paramValue = NULL;
  bool bail = false;
  const char term = request[requestLength];
  char paramValueDecoded[REQUEST_PARAM_VALUE_BUFFER_SIZE];
  char encodedByteStr[3];
  
  if(requestLength > 0)
  {
    request[requestLength] = '\0';
    for(str1 = request; ; str1 = NULL)
    {
      token = strtok_r(str1, "&", &saveptr1);
      if(token == NULL)
      {
        break;
      }
      for(str2 = token; ; str2 = NULL)
      {
        subtoken = strtok_r(str2, "=", &saveptr2);
        if(subtoken == NULL)
        {
          break;
        }
        if(paramName == NULL)
        {
          paramName = subtoken;
          paramNameLength = min(REQUEST_PARAM_NAME_BUFFER_SIZE - 1, strlen(paramName));
        }
        else if(paramValue == NULL)
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
                paramValueDecoded[paramValueDecodedLength++] = static_cast<char>(strtol(encodedByteStr, NULL, 16));
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
          
          paramName = NULL;
          paramValue = NULL;
          paramValueDecodedLength = 0;
        }
      }
    }
    request[requestLength] = term;
  }
  
  return numParams;
}
