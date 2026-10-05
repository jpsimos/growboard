/*
 * author Jacob A Psimos
 */

#ifndef __SD_MIDDLEWARE_H
#define __SD_MIDDLEWARE_H

#include "Middleware.h"
#include <Arduino.h>
#include <elapsedMillis.h>
#include <SPI.h>
#include <SdFat.h>
#include <functional>

// Pin definitions
#define SD_CARD_CS_PIN 10

// Typedefs
typedef std::function<bool(size_t chunkLength, unsigned char chunk[])> ReadFileChunkFunc;

class SdMiddleware : public Middleware
{
  private:
    inline static SdMiddleware* sm_singletonPointer;
    
  public:
    static SdMiddleware* CreateInstance();
    static SdMiddleware* GetInstance();

  private:
    SdMiddleware();
    virtual ~SdMiddleware();

  public:
    virtual bool Initialize();
    virtual void Loop() {}
    virtual int ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream);
    int LoadConfigTxt(char* ssid, char* key, const bool updateWifiMiddleware);
    void AppendLog(const char path[], const char msg[]);
    uint64_t GetFileLength(const char path[]);
    int ReadFile(const char path[], unsigned char data[], const size_t dataSize);
    int ReadFile(const char path[], unsigned char chunkBuffer[], const size_t chunkSize, ReadFileChunkFunc onReadChunk);
    
  private:
    static void FsDateTimeCallback(uint16_t* date, uint16_t* time);
    
  private:
    SdFat32 m_sd;
};


#endif
