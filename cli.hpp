#ifndef __CLI_HPP__
#define __CLI_HPP__

#include <stdint.h>

class SerialPort;

class CLI {
  public:
    CLI(SerialPort* serialPort);

    struct Command {
      const char* name;
      const char* description;
      bool (CLI::*fcn)(void);
    };

    bool printHelp();
    bool printVersion();
    bool memoryDisplayWords();

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

};

#endif
