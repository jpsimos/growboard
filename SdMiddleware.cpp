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
  Middleware(Middleware::AbstractType::SD_CARD),
  m_sd(),
  m_checkCardTimer()
{
    if(m_sd.begin(SdSpiConfig(SD_CARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(12))))
    {
      FsDateTime::setCallback(SdMiddleware::FsDateTimeCallback);
    }
}

SdMiddleware::~SdMiddleware()
{
  m_sd.end();
}

bool SdMiddleware::CheckSdCard()
{
  csd_t csd;
  
  if(m_sd.card()->readCSD(&csd) && csd.capacity() > 0)
  {
    return true;
  }

  return false;
}

void SdMiddleware::Loop()
{
  if(m_checkCardTimer > CARD_CHECK_TIMER)
  {
    m_watchdog = CheckSdCard();
    m_checkCardTimer = 0;
  }
}

int SdMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int result = COMMAND_IGNORE;
  cid_t cid;
  csd_t csd;
  scr_t scr;
  uint32_t ocr;
  bool readInfo;
  
  if(Text::CStrEq("sd status", inputLine))
  {
    if(
      m_sd.card()->readCID(&cid) &&
      m_sd.card()->readCSD(&csd) &&
      m_sd.card()->readSCR(&scr) &&
      m_sd.card()->readOCR(&ocr)
    )
    {
      Serial.print("Card Type: ");
      
      switch(m_sd.card()->type())
      {
        case SD_CARD_TYPE_SD1:
          Serial.println("SD1");
          break;
        case SD_CARD_TYPE_SD2:
          Serial.println("SD2");
          break;
        case SD_CARD_TYPE_SDHC:
          if(csd.capacity() < 70000000)
          {
            Serial.println("SDHC");
          }
          else
          {
            Serial.println("SDXC");
          }
          break;

        default:
          Serial.println("Unknown");
          break;
      }

      Serial.printf("Manufacturer ID: %x\r\n", (int)cid.mid);
      Serial.printf("OEM ID: %x%x\r\n", cid.oid[0], cid.oid[1]);
      Serial.printf("Product ID: %x%x%x%x%x\r\n", cid.pnm[0], cid.pnm[1], cid.pnm[2], cid.pnm[3], cid.pnm[4]);
      Serial.printf("Revision: %d.%d\r\n", (int)cid.prvN(), (int)cid.prvM());
      Serial.printf("Serial Number: %x\r\n", cid.psn());
      Serial.printf("Manufacturing Date: %d/%d\r\n", cid.mdtMonth(), cid.mdtYear());
      
      Serial.print("Size: ");
      Serial.print(0.000512 * csd.capacity());
      Serial.println(" MB");

      Serial.print("Erase Size: ");
      Serial.println((int)csd.eraseSize());

      Serial.print("Erase Single Block: ");
      Serial.println(csd.eraseSingleBlock() ? "true" : "false");

      Serial.print("Data After Erase: ");
      Serial.println(scr.dataAfterErase() ? "0xFF" : "0x00");
      
      result = COMMAND_OK;
    }
    else
    {
      result = COMMAND_ERROR;
    }
  }
  
  return result;
}

bool SdMiddleware::LoadConfigTxt(
  char* ssid,
  const size_t maxSsidBufLen,
  char* key,
  const size_t maxKeyBufLen,
  bool& autoconnect,
  bool& autoreconnect
)
{
  bool hadError;
  int lineLength;
  char* strippedLinePtr;
  char line[CONFIG_FILE_LINE_BUFFER_SIZE];
  File32 configFile;

  hadError = !(ssid != nullptr && key != nullptr && maxSsidBufLen > 0 && maxKeyBufLen > 0);

  if(!hadError && configFile.open("/Config.txt", FILE_READ))
  {
    memset(&line[0], '\0', CONFIG_FILE_LINE_BUFFER_SIZE);
    
    for(lineLength = configFile.fgets(line, sizeof(line) - 1); lineLength > 0 && lineLength < CONFIG_FILE_LINE_BUFFER_SIZE; lineLength = configFile.fgets(line, sizeof(line)))
    {
      Text::CStrStripNewLine(line);
      strippedLinePtr = Text::CStrLStrip(line);
      lineLength = strlen(strippedLinePtr);

      if(lineLength > 0 && lineLength < CONFIG_FILE_LINE_BUFFER_SIZE)
      {
        if(strippedLinePtr[0] == ';')
        {
          continue;
        }
      }
      else
      {
        continue;
      }

      if(lineLength > 5 && line == strstr(line, "ssid="))
      {
        strncpy(ssid, &line[5], maxSsidBufLen);
        ssid[maxSsidBufLen - 1] = '\0';
      }
      else if(lineLength > 4 && line == strstr(line, "key="))
      {
        strncpy(key, &line[4], maxKeyBufLen);
        key[maxKeyBufLen - 1] = '\0';
      }
      else if(Text::CStrEq(line, "autoconnect=true", false))
      {
          autoconnect = true;
      }
      else if(Text::CStrEq(line, "autoreconnect=true", false))
      {
        autoreconnect = true;
      }
      else
      {
        hadError = true;
        break;
      }
    }
    configFile.close();
  }
  else
  {
    hadError = true;
  }
  return !hadError;
}

bool SdMiddleware::LoadAirQualityBaseline(uint16_t& baseline)
{
  File32 airQualityBaselineFile;
  bool hadError = !airQualityBaselineFile.open("/AirQualityBaseline.bin", FILE_READ);
  
  if(!hadError)
  {
    hadError = !(airQualityBaselineFile.read(reinterpret_cast<void*>(&baseline), sizeof(uint16_t)) == sizeof(uint16_t));
    airQualityBaselineFile.close();
  }

  return !hadError;
}

bool SdMiddleware::SaveAirQualityBaseline(const uint16_t baseline)
{
  bool hadError;
  File32 airQualityBaselineFile;

  hadError = !airQualityBaselineFile.open("/AirQualityBaseline.bin", FILE_WRITE);

  if(!hadError)
  {
    hadError = !(airQualityBaselineFile.write(reinterpret_cast<void*>(const_cast<uint16_t*>(&baseline)), sizeof(uint16_t)) == sizeof(uint16_t));
    airQualityBaselineFile.close();
  }

  return !hadError;
}

bool SdMiddleware::AppendLog(const char path[], const char msg[])
{
  File32 file;
  size_t msgLength;
  bool hadError = !(path && msg && nullptr != RtcMiddleware::GetInstance());
  
  if(!hadError)
  {
    msgLength = strlen(msg);
    
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
      else
      {
        hadError = true;
      }
    }
  }

  return !hadError;
}

uint64_t SdMiddleware::GetFileLength(const char path[])
{
  uint64_t length = 0;
  File32 file;
  
  if(path && file.open(path, FILE_READ))
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
  
  if(path && data && dataSize > 0 && file.open(path, FILE_READ))
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
  
  if(path && chunkBuffer && chunkSize > 0 && file.open(path, FILE_READ))
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
    if(date != nullptr)
    {
       *date = FS_DATE(now.year(), now.month(), now.day());
    }
    if(time != nullptr)
    {
      *time = FS_TIME(now.hour(), now.minute(), now.second());
    }
  }
}
