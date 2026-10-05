/*
 * author Jacob A Psimos
 */

#include "RtcMiddleware.h"
#include "Common.h"

RtcMiddleware* RtcMiddleware::CreateInstance()
{
  if(nullptr == RtcMiddleware::sm_singletonPointer)
  {
    RtcMiddleware::sm_singletonPointer = new RtcMiddleware();
  }
  return RtcMiddleware::sm_singletonPointer;
}

RtcMiddleware* RtcMiddleware::GetInstance()
{
  return RtcMiddleware::sm_singletonPointer;
}

RtcMiddleware::RtcMiddleware(const bool interruptEnable) :
  Middleware(Middleware::AbstractType::REAL_TIME_CLOCK),
  m_interruptEnable(interruptEnable),
  m_rtcBoard(new RTC_PCF8523{})
{
  Initialize();
}

RtcMiddleware::~RtcMiddleware()
{
  if(m_interruptEnable)
  {
    detachInterrupt(digitalPinToInterrupt(RTC1_SQWPIN));
  }
  
  if(nullptr != m_rtcBoard)
  {    
    delete m_rtcBoard;
  }
}

bool RtcMiddleware::Initialize()
{
  if(!m_initialized && nullptr != m_rtcBoard)
  {
    if(m_rtcBoard->begin())
    {
        m_initialized = true;
        pinMode(RTC1_SQWPIN, INPUT_PULLUP);
        if(!m_rtcBoard->initialized() || m_rtcBoard->lostPower())
        {
          m_rtcBoard->adjust(DateTime(__DATE__, __TIME__));
        }
        m_rtcBoard->start();
        m_rtcBoard->calibrate(Pcf8523OffsetMode::PCF8523_TwoHours, 8);
        {
          delay(10);
          DateTime now = m_rtcBoard->now();
          uint32_t rs = now.unixtime();
          randomSeed(rs);
        }
        if(m_interruptEnable)
        {
          m_rtcBoard->writeSqwPinMode(PCF8523_SquareWave1HZ);
          attachInterrupt(digitalPinToInterrupt(RTC1_SQWPIN), reinterpret_cast<void(*)()>(RtcMiddleware::HandleIsr), FALLING);
        }
    }
  }
  return m_initialized;
}

DateTime RtcMiddleware::Now()
{
  if(m_initialized)
  {
    return m_rtcBoard->now();
  }
  return DateTime(__DATE__, __TIME__);
}

const char* const RtcMiddleware::NowString()
{
    static char buffer[] = "DDD, DD MMM YYYY hh:mm:ss";
    if(m_initialized)
    {
      m_rtcBoard->now().toString(buffer);
    }
    return static_cast<const char* const>(buffer);
}

int RtcMiddleware::ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream)
{
  int result = COMMAND_IGNORE;
  unsigned int month;
  unsigned int day;
  unsigned int year;
  unsigned int hour;
  unsigned int minute;
  unsigned int second;

  if(Text::streq(inputLine, "clock read"))
  {
    char buffer[] = "DDD, DD MMM YYYY hh:mm:ss";
    m_rtcBoard->now().toString(buffer);
    stream->println(buffer);
    result |= COMMAND_OK;
  }
  else if(6 == sscanf(inputLine, "clock set %u/%u/%u %u:%u:%u", &month, &day, &year, &hour, &minute, &second))
  {
    m_rtcBoard->adjust(DateTime(year, month, day, hour, minute, second));
    result |= COMMAND_OK;
  }
  
  return result;
}

void RtcMiddleware::HandleIsr()
{
  RtcMiddleware* rtcMiddleware = RtcMiddleware::GetInstance();
}
