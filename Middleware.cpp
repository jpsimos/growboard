/*
 * Author Jacob A. Psimos
 */

#include <Arduino.h>
#include "Middleware.h"
#include "Common.h"

Middleware** Middleware::GetMiddlewares()
{
  return sm_middlewarePointers;
}

Middleware::Middleware(const Middleware::AbstractType abstractType) :
  m_abstractType(abstractType),
  m_watchdog(true)
{
}

void Middleware::ForEach(std::function<bool(Middleware*)> callback)
{
  for(size_t i = 0; i < Middleware::NumMiddlewares; i++)
  {
    if(nullptr != Middleware::sm_middlewarePointers[i])
    {
      if(!callback(Middleware::sm_middlewarePointers[i]))
      {
        break;
      }
    }
  }
}

bool Middleware::Register()
{
  bool alreadyRegistered = false;
  Middleware** cursor = nullptr;
  
  for(size_t i = 0; i < Middleware::NumMiddlewares; i++)
  {
    if(nullptr == Middleware::sm_middlewarePointers[i])
    {
      cursor = &Middleware::sm_middlewarePointers[i];
    }
    if(this == Middleware::sm_middlewarePointers[i])
    {
      alreadyRegistered = true;
      break;
    }
  }

  if(!alreadyRegistered && cursor != nullptr)
  {
    *cursor = this;
    return true;
  }

  return false;
}

bool Middleware::GetWatchdog() const
{
  return m_watchdog;
}

bool Middleware::Unregister()
{
  for(size_t i = 0; i < Middleware::NumMiddlewares; i++)
  {
    if(this == Middleware::sm_middlewarePointers[i])
    {
      Middleware::sm_middlewarePointers[i] = nullptr;
      return true;
    }
  }

  return false;
}

Middleware::AbstractType Middleware::GetAbstractType() const
{
  return m_abstractType;
}

int Middleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  return COMMAND_IGNORE;
}

int Middleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  // Output written to the client should be in JSON.
  return COMMAND_IGNORE;
}
