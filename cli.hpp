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

class SerialPort;
class Parameters;
class RTOS;

class CLI {
  public:
    CLI(SerialPort* serialPort, Parameters* parameters, const RTOS & rtos);

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
    bool setParameter();
    bool reset();
    bool resources();

    // FreeRTOS task for transmitting characters
    void task(void *params);

  private:

    void execute();
    void putPrompt();

    // Can parse hex or unsigned integer numbers, return true if succesful,
    // if false will not modify the result
    bool parseInt(const char* const input, uint32_t* result,
                  bool* isSigned = NULL);

    char buf[64];
    uint8_t next;

    SerialPort* p;
    Parameters* parameters;
    const RTOS & rtos;

};

#endif
