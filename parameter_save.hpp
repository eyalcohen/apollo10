/*
 * parameter_save.hpp
 * Author: Eyal Cohen
 *
 * Stores ROM parameters in EEPROM.  Note!  Single-threaded, if this
 * needs to be called from interrupts or tasks for any reason (and why
 * would it?) then add some OS exclusion guards.
 */

#ifndef __PARAMETER_SAVE_HPP__
#define __PARAMETER_SAVE_HPP__

#include "common.hpp"
#include "parameters.hpp"

class ParameterSave {
  public:
    ParameterSave();

    // All functions returns false if failed
    static bool init(char err[ERR_BYTES]);
    static bool save(const Parameters* parameters, char err[ERR_BYTES]); 
    static bool load(Parameters* parameters, char err[ERR_BYTES]); 
    static bool erase(char err[ERR_BYTES]);

  private:
    struct Header {
      Parameters::ParameterIndex parameterCount;
      uint32_t crc32;
      uint8_t version;
    };

    struct ParamHeader {
      uint8_t nameLen;
      uint32_t value;
      Parameters::Type type;
    };

    enum { ParameterVersion = 0 };
    enum { ParamsCanSave = 32 };

    // round a number to the nearest 4th
    static uint32_t align32(uint32_t v) {
      uint32_t r = v % 4;  
      return r ? v + (4 - r) : v;
    }
};

#endif
