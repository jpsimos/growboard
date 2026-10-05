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
  m_initialized(false)
{
}

bool Middleware::Register()
{
  if(Middleware::sm_middlewarePointerCursor < Middleware::NumMiddlewares)
  {
    Middleware::sm_middlewarePointers[Middleware::sm_middlewarePointerCursor++] = this;
    return true;
  }
  return false;
}

Middleware::AbstractType Middleware::GetAbstractType() const
{
  return m_abstractType;
}

bool Middleware::Initialize()
{
  return m_initialized;
}

int Middleware::ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream)
{
  return COMMAND_IGNORE;
}
