#include "cli.hpp"

#include <string.h>

#include "common.hpp"
#include "serialport.hpp"

// Command table for CLI
static CLI::Command commandTable[] = {
  {"version", &CLI::printVersion},
  {"help", &CLI::printHelp},
  {"mdw", &CLI::memoryDisplayWords},
};

// Constructor
CLI::CLI(SerialPort* serialPort) : next(0), serialPort(serialPort) {
}

// FreeRTOS task that reads from queue and executes commands
void CLI::task(void *params) {
  putPrompt();
  while (1) {
    char c = serialPort->get((uint32_t)-1);
    if (next != sizeof(buf)) {
      buf[next++] = c;
    }
    switch (c) {
      case '\n':
      case '\r':
        execute();
        break;
      default:
        serialPort->put(c);
        break;
    }
  }
}

// On enter
void CLI::execute() {
  serialPort->putLine("");
  next = 0;

  const char* first = strtok(buf,  " ,\n,\r");

  for (uint32_t i = 0; i < ARRAY_LENGTH(commandTable); i++) {
    if (strcmp(first, commandTable[i].name) == 0) {
      void (CLI::*ptr)(void) = commandTable[i].fcn;
      (*this.*ptr)();
    }
  }
  putPrompt();
}

void CLI::putPrompt() {
  serialPort->put('>');
  serialPort->put(' ');
}

void CLI::printHelp() {
  serialPort->putLine("Commands");
  serialPort->putLine("help: Help for you");
  serialPort->putLine("version: Version");
}

void CLI::printVersion() {
  serialPort->putLine("Version ##.##.##");
}

void CLI::memoryDisplayWords() {
  
}
