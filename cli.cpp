#include "cli.hpp"
#include "serialport.hpp"
#include <string.h>

extern CLI cli;
extern SerialPort serialPort;

void cliTask(void *params) {
  cli.task(params);
}

CLI::CLI() : next(0) {

}

void CLI::task(void *params) {
  putPrompt();
  while (1) {
    char c = serialPort.get((uint32_t)-1);
    if (next != sizeof(buf)) {
      buf[next++] = c;
    }
    switch (c) {
      case '\n':
      case '\r':
        execute();
        break;
      default:
        serialPort.put(c);
        break;
    }
  }
}

void CLI::execute() {
  serialPort.putLine("");
  next = 0;

  const char* first = strtok(buf,  " ,\n,\r");

  if (strcmp(first, "version") == 0) {
    serialPort.putLine("Version ##.##.##");
  } else if (strcmp(first, "help") == 0) {
    printHelp();
  }
  putPrompt();
}

void CLI::putPrompt() {
  serialPort.put('>');
  serialPort.put(' ');
}

void CLI::printHelp() {
  serialPort.putLine("Commands");
  serialPort.putLine("help: Help for you");
  serialPort.putLine("version: Version");
}
