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

class SerialPort;
class Parameters;

class CLI {
  public:
    CLI(SerialPort* serialPort, Parameters* parameters);

    struct Command {
      const char* name;
      const char* description;
      bool (CLI::*fcn)(void);
    };

    bool printHelp();
    bool printVersion();
    // mdw gets data in addresses and displays them
    bool memoryDisplayWords();
    bool getParameter();

    // FreeRTOS task for transmitting characters
    void task(void *params);

  private:

    void execute();
    void putPrompt();

    // Can parse hex or unsigned integer numbers, return true if succesful,
    // if false will not modify the result
    bool parseUnsignedInt(const char* input, uint32_t* result);

    char buf[64];
    uint8_t next;

    SerialPort* p;
    Parameters* parameters;

};

#endif
