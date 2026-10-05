/*
 * Author Jacob A. Psimos
 */

#ifndef __MIDDLEWARE_HPP
#define __MIDDLEWARE_HPP

#include <Arduino.h>

// Define integer results for ProcessSerialCommand abstraction.
#define COMMAND_IGNORE    0            // Must be zero. Do not change.
#define COMMAND_OK        1
#define COMMAND_LAST      2
#define COMMAND_ERROR     4

class Middleware
{
  public:
    static constexpr const size_t MaxCommandLengthInclTerm = 128;
    static constexpr const size_t NumMiddlewares = 4;
    
    enum class AbstractType : int
    {
      NONE,
      REAL_TIME_CLOCK,
      SD_CARD,
      WIFI_CARD,
      HYGROMETER_SENSOR,
      MAX_VALUE
    };

  private:
    inline static Middleware* sm_middlewarePointers[Middleware::NumMiddlewares] = { nullptr };
    inline static size_t sm_middlewarePointerCursor = 0;
    
  public:
    static Middleware** GetMiddlewares();
    bool Register();
    Middleware::AbstractType GetAbstractType() const;
    virtual bool Initialize();
    virtual void Loop() {};
    virtual int ProcessSerialCommands(const char inputLine[], ssize_t inputLineLength, Stream* stream);

  protected:
    Middleware(const Middleware::AbstractType abstractType);
    virtual ~Middleware() = default;
    const Middleware::AbstractType m_abstractType;
    bool m_initialized;
};

#endif
