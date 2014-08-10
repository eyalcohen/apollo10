#ifndef __CLI_HPP__
#define __CLI_HPP__

#include <stdint.h>

extern "C" {
  extern void cliTask(void *params);
}

class CLI {
  public:
    CLI();

    struct Command {
      const char* name;
      void (CLI::*fcn)(void);
    };

    void printHelp();
    void printVersion();
    void memoryDisplayWords();

    // FreeRTOS task for transmitting characters
    void task(void *params);

  private:

    void execute();
    void putPrompt();

    char buf[256];
    uint8_t next;

};

#endif
