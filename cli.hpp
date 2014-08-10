#ifndef __CLI_HPP__
#define __CLI_HPP__

#include <stdint.h>

extern "C" {
  extern void cliTask(void *params);
}

class CLI {
  public:
    CLI();

    // FreeRTOS task for transmitting characters
    void task(void *params);

  private:

    void execute();
    void putPrompt();
    void printHelp();

    char buf[256];
    uint8_t next;

};

#endif
