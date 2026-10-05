/*
 * Author Jacob A. Psimos
 */

#include "Middleware.h"
#include "RtcMiddleware.h"
#include "SdMiddleware.h"
#include "WifiMiddleware.h"
#include "HygrometerMiddleware.h"
#include "Common.h"

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  RtcMiddleware::CreateInstance()->Register();
  SdMiddleware::CreateInstance()->Register();
  WifiMiddleware::CreateInstance()->Register();
  //HygrometerMiddleware::CreateInstance()->Register();
}

void loop()
{
  char command[Middleware::MaxCommandLengthInclTerm];
  ssize_t commandLength;

  for(size_t middlewareIndex = 0; middlewareIndex < Middleware::NumMiddlewares; middlewareIndex++)
  {
      Middleware* middlewarePointer = Middleware::GetMiddlewares()[middlewareIndex];
      
      if(nullptr != middlewarePointer)
      {
        middlewarePointer->Loop();
     }
  }

  commandLength = Text::ReadLine(command, Middleware::MaxCommandLengthInclTerm, IO::InOutStream);

  if(commandLength > 0)
  {
    Serial.print("Received: ");
    Serial.println(command);
    
    for(size_t middlewareIndex = 0; middlewareIndex < Middleware::NumMiddlewares; middlewareIndex++)
    {
      Middleware* middlewarePointer = Middleware::GetMiddlewares()[middlewareIndex];

      if(nullptr != middlewarePointer)
      {
        middlewarePointer->ProcessSerialCommands(command, commandLength, &Serial);
     }
    }
  }
}
