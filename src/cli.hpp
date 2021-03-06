/*
 * cli.hpp
 * Author: Eyal Cohen
 *
 * A command line interface with some basic functionality.  Requires
 * a port interface of some sort
 */


#ifndef __CLI_HPP__
#define __CLI_HPP__

#include <stdint.h>
#include <stddef.h>
#include "parameters.hpp"

// Forward declarations
class SerialPort;
class RTOS;

class CLI {
  public:
    CLI(SerialPort* serialPort, Parameters* parameters, const RTOS & rtos);

    struct Command {
      const char* name;
      const char* description;
      bool (CLI::*fcn)(void);
    };

    // CLI Commands
    bool printHelp();
    bool printVersion();
    // mdw gets data in addresses and displays them
    bool memoryDisplayWords();
    bool getParameter();
    bool setParameter();
    bool reset();
    bool stats();
    
    // parameter management
    bool save();
    bool load();
    bool erase();

    // FreeRTOS task for transmitting characters
    void task(void *params);

  private:

    void execute();
    void putPrompt() const;

    void printParam(const Parameters::ParameterGet* param) const;

    // Parse a float number (supports x.x only)
    bool parseFloat(const char* input, float* result) const;

    // Can parse hex or unsigned integer numbers, return true if succesful,
    // if false will not modify the result.  Will parse until a terminating
    // character is found
    bool parseInt(const char* input, uint32_t* result,
                  bool* isNegative = NULL) const;

    char buf[64];
    uint8_t next;

    SerialPort* p;
    Parameters* parameters;
    const RTOS & rtos;

};

#endif
