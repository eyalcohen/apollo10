#include "cli.hpp"

#include <string.h>

#include "common.hpp"
#include "serialport.hpp"
#include "parameters.hpp"
#include "rtos.hpp"

// Command table for CLI
static CLI::Command commandTable[] = {
  {"version", "Displays version information", &CLI::printVersion},
  {"help", "This help menu", &CLI::printHelp},
  {"mdw", "Display data in addresses. Usage: mdw [address] [count]",
    &CLI::memoryDisplayWords},
  {"get", "Gets a parameter.  Usage: get [string]",
    &CLI::getParameter},
  {"set", "Sets a parameter.  Usage: set paramnumber string",
    &CLI::setParameter},
  {"reset", "Reset the system",
    &CLI::reset},
};

// Constructor
CLI::CLI(SerialPort* serialPort, Parameters* parameters)
  : next(0), p(serialPort), parameters(parameters) {
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
  if (!parseInt(nextArg, &address))
    return false;
  address = address - (address % 4);

  // Get count, or default to 1
  uint32_t count = 1;
  nextArg = strtok(NULL, " ,\n,\r");
  parseInt(nextArg, &count);

  // Print memory contents
  while(count--) {
    p->printf("%#08X -> %#08x\n", address, *(uint32_t*)address);
    address += 4;
  }

  return true;
}

bool CLI::getParameter() {
  const char* nextArg = strtok(NULL, " ,\n,\r");
  Parameters::ResultsIterator it;
  if (nextArg) {
    parameters->get(nextArg, &it);
  } else {
    parameters->get("", &it);
  }

  for (it.reset(); it.complete(); ++it) {
    Parameters::ParameterGet results = it.val();
    p->printf("[%02d] %s = ", results.index, results.name, results.description);
    if (results.type == Parameters::Int8
        || results.type == Parameters::Int16
        || results.type == Parameters::Int32 ) {
      p->printf("%d", (int32_t)results.valueUntyped);
    } else {
      p->printf("%u", (uint32_t)results.valueUntyped);
    }
    p->printf("\n");
  }
  return true;
}

bool CLI::setParameter() {
  const char* paramString = strtok(NULL, " ,\n,\r");
  if (!paramString)
    return false;
  const char* valueString = strtok(NULL, " ,\n,\r");
  if (!valueString)
    return false;
  bool isSigned;

  uint32_t param, val;
  if (!parseInt(paramString, &param)) {
    p->printf("Could not understand param %s\n", paramString);
    return false;
  }

  if (!parseInt(valueString, &val, &isSigned)) {
    p->printf("Could not understand value %s\n", valueString);
    return false;
  }

  char err[32];
  if (!parameters->set(param, isSigned ? (int32_t)val: (uint32_t)val, err)) {
    p->printf("Error %s\n", err);
  }

  return true;
}

#include "driverlib/sysctl.h"
#include "FreeRTOS.h"
#include "task.h"

bool CLI::reset() {
  p->printf("Resetting...\n");
  RTOS::sleep_ms(200);
  SysCtlReset();
  while (true) {

  }
  return false;
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
      if (!result) {
        p->printf("  %s - %s\n", commandTable[i].name,
                                 commandTable[i].description);
      }
      next = 0;
      break;
    }
  }
  putPrompt();
}

void CLI::putPrompt() {
  p->put('>');
  p->put(' ');
}
bool CLI::parseInt(const char* const input, uint32_t* result,
                   bool* isSigned) {

  uint32_t num = 0;
  bool hex = false;
  const char* iterator = input;

  if (*iterator == '0' && (*(iterator+1) == 'x' || *(iterator+1) == 'X')) {
    hex = true;
    iterator += 2;
  } else if (*iterator == '-') {
    iterator++;
    if (!isSigned)
      *isSigned = true;
  }

  for(; *iterator; iterator++) {
    num *= hex ? 16 : 10;
    if (hex && *iterator >= 'a' && *iterator <= 'f') {
      num += (*iterator - 'a' + 10);
    } else if (hex && *iterator >= 'A' && *iterator <= 'F') {
      num += (*iterator - 'A' + 10);
    } else if (*iterator >= '0' && *iterator <= '9'){
      num += *iterator - '0';
    } else {
      return false;
    }
  }

  *result = num;
  return true;

}
