#include "cli.hpp"

#include <string.h>

#include "common.hpp"
#include "serialport.hpp"

// Command table for CLI
static CLI::Command commandTable[] = {
  {"version", "Displays version information", &CLI::printVersion},
  {"help", "This help menu", &CLI::printHelp},
  {"mdw", "Display data in addresses. Usage mdw [address] [count]", &CLI::memoryDisplayWords},
};

// Constructor
CLI::CLI(SerialPort* serialPort) : next(0), p(serialPort) {
}

// FreeRTOS task that reads from queue and executes commands
void CLI::task(void *params) {
  putPrompt();
  while (1) {
    char c = p->get((uint32_t)-1);
    if (next != sizeof(buf)) {
      buf[next++] = c;
    }
    switch (c) {
      case '\n':
      case '\r':
        execute();
        break;
      default:
        p->put(c);
        break;
    }
  }
}

bool CLI::printHelp() {
  p->putLine("CLI commands");
  for (uint32_t i = 0; i < ARRAY_LENGTH(commandTable); i++) {
    p->printf("  %s - %s\n", commandTable[i].name, commandTable[i].description);
  }
  return true;
}

bool CLI::printVersion() {
  p->putLine("Version ##.##.##");
  return true;
}

bool CLI::memoryDisplayWords() {
  const char* nextArg = strtok(NULL, " ,\n,\r");
  if (!nextArg) return false;

  // Get address
  uint32_t address;
  if (!parseUnsignedInt(nextArg, &address))
    return false;
  address = address - (address % 4);

  // Get count, or default to 1
  uint32_t count = 1;
  nextArg = strtok(NULL, " ,\n,\r");
  parseUnsignedInt(nextArg, &count);

  // Print memory contents
  while(count--) {
    p->printf("%#08X -> %#08x\n", address, *(uint32_t*)address);
    address += 4;
  }

  return true;
}

// On enter
void CLI::execute() {
  p->putLine("");
  next = 0;

  const char* first = strtok(buf,  " ,\n,\r");

  // Arcane C++ syntax to call a member function pointer
  for (uint32_t i = 0; i < ARRAY_LENGTH(commandTable); i++) {
    if (strcmp(first, commandTable[i].name) == 0) {
      bool (CLI::*ptr)(void) = commandTable[i].fcn;
      bool result = (*this.*ptr)();
      if (!result)
        p->printf("  %s - %s\n", commandTable[i].name,
                                 commandTable[i].description);
    }
  }
  putPrompt();
}

void CLI::putPrompt() {
  p->put('>');
  p->put(' ');
}
bool CLI::parseUnsignedInt(const char* input, uint32_t* result) {

  uint32_t num = 0;
  bool hex = false;
  if (*input == '0' && (*(input+1) == 'x' || *(input+1) == 'X')) {
    hex = true;
    input += 2;
  }

  for(; *input; input++) {
    num *= hex ? 16 : 10;
    if (hex && *input >= 'a' && *input <= 'f') {
      num += (*input - 'a' + 10);
    } else if (hex && *input >= 'A' && *input <= 'F') {
      num += (*input - 'A' + 10);
    } else if (*input >= '0' && *input <= '9'){
      num += *input - '0';
    } else {
      return false;
    }
  }

  *result = num;
  return true;

}
