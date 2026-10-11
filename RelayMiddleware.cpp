/*
 * author Jacob A Psimos
 */

#include "RelayMiddleware.h"
#include "Common.h"

RelayMiddleware* RelayMiddleware::CreateInstance()
{
  if(nullptr == RelayMiddleware::sm_singletonPointer)
  {
    RelayMiddleware::sm_singletonPointer = new RelayMiddleware();
  }
  return RelayMiddleware::sm_singletonPointer;
}

RelayMiddleware* RelayMiddleware::GetInstance()
{
  return RelayMiddleware::sm_singletonPointer;
}

RelayMiddleware::RelayMiddleware() :
  Middleware(Middleware::AbstractType::RELAY_CONTROLLER),
  m_debounceTimer(),
  m_command(RELAY_CMD_NONE)
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

RelayMiddleware::~RelayMiddleware()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void RelayMiddleware::Command(const int command)
{
  switch(command)
  {
    case RELAY_CMD_ON:
      m_command = RELAY_CMD_ON;
      break;
    case RELAY_CMD_OFF:
      m_command = RELAY_CMD_OFF;
      break;
  }
}

void RelayMiddleware::Loop()
{
  if(m_debounceTimer >= RELAY_DEBOUNCE_TIME)
  {
    switch(m_command)
    {
      case RELAY_CMD_ON:
        digitalWrite(RELAY_PIN, HIGH);
        m_debounceTimer = 0;
        m_command = RELAY_CMD_NONE;
        break;
      case RELAY_CMD_OFF:
        digitalWrite(RELAY_PIN, LOW);
        m_debounceTimer = 0;
        m_command = RELAY_CMD_NONE;
        break;
    }
  }
}

int RelayMiddleware::ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client)
{
  int commandResult = COMMAND_IGNORE;

  if(Text::CStrEq(inputLine, "relay on"))
  {
      Command(RELAY_CMD_ON);
      client.println("{\"relay\":\"on\"}");
      commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "relay off"))
  {
    Command(RELAY_CMD_OFF);
    client.println("{\"relay\":\"off\"}");
    commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "relay status"))
  {
    Command(RELAY_CMD_OFF);
    client.print("{\"relay\":\"");
    client.print(digitalRead(RELAY_PIN) == HIGH ? "on" : "off");
    client.println("\"}");
    commandResult = COMMAND_OK;
  }

  return commandResult;
}

int RelayMiddleware::ProcessSerialCommand(const char inputLine[], size_t inputLineLength)
{
  int commandResult = COMMAND_IGNORE;

  if(Text::CStrEq(inputLine, "relay on"))
  {
      Command(RELAY_CMD_ON);
      Serial.println("Relay on");
      commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "relay off"))
  {
    Command(RELAY_CMD_OFF);
    Serial.println("Relay off");
    commandResult = COMMAND_OK;
  }
  else if(Text::CStrEq(inputLine, "relay status"))
  {
    Serial.print("Relay ");
    Serial.println(digitalRead(RELAY_PIN) == HIGH ? "on" : "off");
    commandResult = COMMAND_OK;
  }

  return commandResult;
}
 
