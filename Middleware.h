/*
 * Author Jacob A. Psimos
 */

#ifndef __MIDDLEWARE_HPP
#define __MIDDLEWARE_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include <functional>

// Define integer results for ProcessSerialCommand abstraction.
#define COMMAND_IGNORE    0            // Must be zero. Do not change.
#define COMMAND_OK        1
#define COMMAND_LAST      2
#define COMMAND_ERROR     4

#define MIDDLEWARE_TYPES \
  MIDDLEWARE_TYPE_DEF(BASE = 0) \
  MIDDLEWARE_TYPE_DEF(REAL_TIME_CLOCK) \
  MIDDLEWARE_TYPE_DEF(SD_CARD) \
  MIDDLEWARE_TYPE_DEF(WIFI_CARD) \
  MIDDLEWARE_TYPE_DEF(HYGROMETER_SENSOR) \
  MIDDLEWARE_TYPE_DEF(CCS811_SENSOR) \
  MIDDLEWARE_TYPE_DEF(RELAY_CONTROLLER)

class Middleware
{
  public:
    static constexpr const size_t MaxCommandLengthInclTerm = 128;
    static constexpr const size_t NumMiddlewares = 10;
    
    enum class AbstractType : int
    {
      #define MIDDLEWARE_TYPE_DEF(type) type,
        MIDDLEWARE_TYPES
      #undef MIDDLEWARE_TYPE_DEF
      MAX_VALUE
    };

    static constexpr const char* MiddlewareAbstractTypeNames[] = {
      #define MIDDLEWARE_TYPE_DEF(type) #type,
        MIDDLEWARE_TYPES
      #undef MIDDLEWARE_TYPE_DEF
    };

  private:
    inline static Middleware* sm_middlewarePointers[Middleware::NumMiddlewares] = { nullptr };
    inline static size_t sm_middlewarePointerCursor = 0;
    
  public:
    static Middleware** GetMiddlewares();
    static void ForEach(std::function<bool(Middleware*)> callback);
    bool GetWatchdog() const;
    bool Register();
    bool Unregister();
    Middleware::AbstractType GetAbstractType() const;
    virtual void Loop() {};
    virtual int ProcessSerialCommand(const char inputLine[], size_t inputLineLength);
    virtual int ProcessRestfulApiCommand(const char inputLine[], size_t inputLineLength, WiFiClient& client);

  protected:
    Middleware(const Middleware::AbstractType abstractType);
    virtual ~Middleware() = default;
    const Middleware::AbstractType m_abstractType;
    bool m_watchdog;
};

#endif
