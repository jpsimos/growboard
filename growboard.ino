/*
 * Author Jacob A. Psimos
 */

#include "Common.h"
#include "Middleware.h"
#include "RtcMiddleware.h"
#include "SdMiddleware.h"
#include "WifiMiddleware.h"
#include "HygrometerMiddleware.h"
#include "RelayMiddleware.h"
#include "AirQualityMiddleware.h"
#include "LightMiddleware.h"
#include <Adafruit_SleepyDog.h>

void setup()
{
  Watchdog.enable(10000, false);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  RtcMiddleware::CreateInstance()->Register();
  SdMiddleware::CreateInstance()->Register();
  WifiMiddleware::CreateInstance()->Register();
  RelayMiddleware::CreateInstance()->Register();
  HygrometerMiddleware::CreateInstance()->Register();
  AirQualityMiddleware::CreateInstance()->Register();
  LightMiddleware::CreateInstance()->Register();
}

void loop()
{
  char command[Middleware::MaxCommandLengthInclTerm];
  size_t commandLength;
  bool watchdog;
  
  Middleware::ForEach(
    [=](Middleware* middlewarePtr) -> bool {
      middlewarePtr->Loop();
      return true;
    }
  );

  commandLength = Text::ReadLine(command, Middleware::MaxCommandLengthInclTerm);

  if(commandLength > 0)
  {
     Middleware::ForEach(
      [=](Middleware* middlewarePtr) -> bool {
        const int commandResult = middlewarePtr->ProcessSerialCommand(command, commandLength);
        if(COMMAND_OK == commandResult)
        {
          Serial.println("OK");
        }
        else if(COMMAND_ERROR == commandResult)
        {
          Serial.println("ERR");
        }
        return true;
      }
    );
  }

  watchdog = true;

  Middleware::ForEach(
    [&](Middleware* middlewarePtr) -> bool {
      if(!middlewarePtr->GetWatchdog())
      {
        watchdog = false;
        return false;
      }
      return true;
    }
  );

  if(watchdog)
  {
    Watchdog.reset();
  }
}
