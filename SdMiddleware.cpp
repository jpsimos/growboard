/*
 * author Jacob A Psimos
 */

#include "WifiMiddleware.h"
#include "RtcMiddleware.h"
#include "SdMiddleware.h"
#include "Common.h"

SdMiddleware* SdMiddleware::CreateInstance()
{
  if(nullptr == SdMiddleware::sm_singletonPointer)
  {
    SdMiddleware::sm_singletonPointer = new SdMiddleware();
  }
  return SdMiddleware::sm_singletonPointer;
}

SdMiddleware* SdMiddleware::GetInstance()
{
  return SdMiddleware::sm_singletonPointer;
}

SdMiddleware::SdMiddleware() :
  Middleware(Middleware::AbstractType::SD_CARD)
{
  Initialize();
}

SdMiddleware::~SdMiddleware()
{
  if(m_initialized)
  {
    m_sd.end();
  }
}

bool SdMiddleware::Initialize()
{
  if(!m_initialized)
  {
    if(m_sd.begin(SdSpiConfig(SD_CARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(12))))
    {
      m_initialized = true;
      digitalWrite(LED_BUILTIN, HIGH);
      FsDateTime::setCallback(SdMiddleware::FsDateTimeCallback);
    }
  }
  return m_initialized;
}

int SdMiddleware::ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream)
{
  int result = COMMAND_IGNORE;

  if(Text::streq("sd test", inputLine))
  {
    Serial.printf("LoadConfigTxt = %d\r\n", LoadConfigTxt(NULL, NULL, false));
    result = COMMAND_OK;
  }
  
  return result;
}

int SdMiddleware::LoadConfigTxt(char* ssid, char* key, const bool updateWifiMiddleware)
{
  int result = 0;
  char line[128];
  File32 configFile;

  if(configFile.open("/Config.txt", FILE_READ))
  {
    line[sizeof(line) - 1] = '\0';
    for(int lineLength = configFile.fgets(line, sizeof(line) - 1); lineLength > 0; lineLength = configFile.fgets(line, sizeof(line)))
    {
      Text::stripNewLine(line);
      lineLength = strlen(line);
      bool skip = false;
      
      for(char* comment = line; comment < &line[lineLength]; comment++)
      {
        if(isblank(*comment))
        {
          continue;
        }
        else if(*comment == ';')
        {
          skip = true;
          break;
        }
      }
      if(skip)
      {
        continue;
      }
      if(ssid != NULL && lineLength > 5 && line == strstr(line, "ssid="))
      {
        strncpy(ssid, &line[5], 33);
        ssid[32] = '\0';
        result++;
      }
      else if(key != NULL && lineLength > 4 && line == strstr(line, "key="))
      {
        strncpy(key, &line[4], 33);
        key[32] = '\0';
        result++;
      }
      else if(updateWifiMiddleware)
      {
        if(Text::streq(line, "autoconnect=true"))
        {
          WifiMiddleware::GetInstance()->SetAutoconnect(true);
        }
        else if(Text::streq(line, "autoconnect=false"))
        {
          WifiMiddleware::GetInstance()->SetAutoconnect(false);
        }
        else if(Text::streq(line, "http=disable"))
        {
          WifiMiddleware::GetInstance()->SetHttpEnabled(false);
        }
        else if(Text::streq(line, "http=enable"))
        {
          WifiMiddleware::GetInstance()->SetHttpEnabled(true);
        }
      }
    }
    configFile.close();
  }
  return result;
}

void SdMiddleware::AppendLog(const char path[], const char msg[])
{
  File32 file;
  const size_t msgLength = strlen(msg);
  if(msgLength > 0)
  {
    if(!file.open(path, FILE_APPEND))
    {
      file.open(path, FILE_WRITE);
    }
    if(file.isOpen())
    {
      const char* const now = RtcMiddleware::GetInstance()->NowString();
      file.write(now, strlen(now));
      file.write("\t", 1);
      file.write(msg, msgLength);
      if(msg[msgLength - 1] != '\n')
      {
        file.write("\r\n", 2);
      }
      file.close();
    }
  }
}

uint64_t SdMiddleware::GetFileLength(const char path[])
{
  uint64_t length = 0;
  File32 file;
  if(file.open(path, FILE_READ))
  {
    length = file.fileSize();
    file.close();
  }
  return length;
}

int SdMiddleware::ReadFile(const char path[], unsigned char data[], const size_t dataSize)
{
  int numBytes = 0;
  File32 file;
  if(data && dataSize > 0 && file.open(path, FILE_READ))
  {
    numBytes = file.read(data, dataSize);
    file.close();
  }
  return numBytes;
}

int SdMiddleware::ReadFile(const char path[], unsigned char chunkBuffer[], const size_t chunkSize, ReadFileChunkFunc onReadChunk)
{
  int numBytes = 0;
  File32 file;
  
  if(file.open(path, FILE_READ))
  {
    while(numBytes < file.fileSize())
    {
      int chunkLength = file.read(chunkBuffer, chunkSize);
      if(chunkLength > 0)
      {
        if(!onReadChunk(chunkLength, chunkBuffer))
        {
          break;
        }
        numBytes += chunkLength;
      }
      else
      {
        break;
      }
    }
    file.close();
  }

  return numBytes;
}

void SdMiddleware::FsDateTimeCallback(uint16_t* date, uint16_t* time)
{
  RtcMiddleware* rtcMiddleware = RtcMiddleware::GetInstance();

  if(nullptr != rtcMiddleware)
  {
    DateTime now = rtcMiddleware->Now();
    if(date != NULL)
    {
       *date = FS_DATE(now.year(), now.month(), now.day());
    }
    if(time != NULL)
    {
      *time = FS_TIME(now.hour(), now.minute(), now.second());
    }
  }
}
