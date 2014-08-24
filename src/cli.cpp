#include "cli.hpp"

#include <string.h>

#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

#include "common.hpp"
#include "serialport.hpp"
#include "rtos.hpp"
#include "parameter_save.hpp"
#include "timer.hpp"

// Command table for CLI
static CLI::Command commandTable[] = {
  {"version", "Displays version information", &CLI::printVersion},
  {"help", "This help menu", &CLI::printHelp},
  {"mdw", "Display data in addresses. Usage: mdw [address] [count]",
    &CLI::memoryDisplayWords},
  {"get", "Gets a parameter.  Usage: get [paramName|paramNumber]",
    &CLI::getParameter},
  {"set", "Sets a parameter.  Usage: set paramNumber|paramName string",
    &CLI::setParameter},
  {"reset", "Reset the system", &CLI::reset},
  {"stats", "OS and run-time information", &CLI::stats},
  {"save", "Save parameters to EEPROM", &CLI::save},
  {"load", "Load parameters from EEPROM", &CLI::load},
  {"erase", "Erase parameters from EEPROM", &CLI::erase},
};

// Constructor
CLI::CLI(SerialPort* serialPort, Parameters* parameters, const RTOS & rtos)
  : next(0), p(serialPort), parameters(parameters), rtos(rtos) {
}

// FreeRTOS task that reads from queue and executes commands
void CLI::task(void *params) {

  // CLI initialization
  char err[32];
  if (!ParameterSave::init(err)) {
    p->printf("EEPROM initialization error: %s\n", err);
  }
  if (!ParameterSave::load(parameters, err)) {
    p->printf("EEPROM load error: %s\n", err);
  } else {
    p->printf("Loaded parameters from EEPROM\n");
  }
  putPrompt();

  while (true) {
    char c = p->get((uint32_t)-1);
    //p->printf("\nchar:%d\n", c);
    switch (c) {
      //backspace
      /*
      case 127:
        if (next != 0)
          next--;
        break;
        p->put(0x1b);
        p->printf("[1D");
        p->put(0x1b);
        p->printf("[K");
      */
      case '\n':
      case '\r':
        if (next != 0) {
          buf[next++] = '\0';
          execute();
        } else {
          p->putLine();
          putPrompt();
        }
        break;
      default:
        if (next != sizeof(buf) - 1) {
          buf[next++] = c;
        }
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
  Parameters::ParameterGet param;
  uint32_t paramNumber = (uint32_t)-1;

  // First try getting the param number
  if (parseInt(nextArg, &paramNumber))
    parameters->get(paramNumber, &param);
  else if (nextArg) {
    parameters->get(nextArg, &it);
  } else {
    parameters->get("", &it);
  }

  if (paramNumber != (uint32_t)-1) {
    printParam(&param);
  } else {
    for (it.reset(); it.complete(); ++it) {
      Parameters::ParameterGet result = it.val();
      printParam(&result);
    }
  }
  return true;
}

void CLI::printParam(const Parameters::ParameterGet* param) {
  p->printf("[%02d] %s = ", param->index, param->name, param->description);
  if (param->type == Parameters::Int8
      || param->type == Parameters::Int16
      || param->type == Parameters::Int32 ) {
    p->printf("%d", (int32_t)param->valueUntyped);
  } else {
    p->printf("%u", (uint32_t)param->valueUntyped);
  }
  p->printf("\n");
}


bool CLI::setParameter() {
  const char* paramString = strtok(NULL, " ,\n,\r");
  if (!paramString)
    return false;
  const char* valueString = strtok(NULL, " ,\n,\r");
  if (!valueString)
    return false;
  bool isNegative;

  uint32_t param, val;

  bool paramIsNum = true;
  if (!parseInt(paramString, &param)) {
    paramIsNum = false;
  }

  if (!parseInt(valueString, &val, &isNegative)) {
    p->printf("\"%s\" is not a command\n", valueString);
    return false;
  }

  char err[32];
  if (paramIsNum) {
    if (!parameters->set(param, isNegative ? -((int32_t)val): (uint32_t)val, err)) {
      p->printf("Error %s\n", err);
      return false;
    }

    // Print parameter from integer
    Parameters::ParameterGet paramGet;
    parameters->get(param, &paramGet);
    printParam(&paramGet);

  } else {
    if (!parameters->set(paramString, isNegative ? (int32_t)val: (uint32_t)val, err)) {
      p->printf("Error %s\n", err);
      return false;
    }

    // Print parameter from string name
    Parameters::ResultsIterator it;
    parameters->get(paramString, &it);
    for (it.reset(); it.complete(); ++it) {
      Parameters::ParameterGet result = it.val();
      printParam(&result);
    }
  }

  return true;
}

bool CLI::reset() {
  p->printf("Resetting...\n");
  RTOS::sleep_ms(200);
  SysCtlReset();
  while (true) {

  }
  return false;
}

bool CLI::stats() {

  RTOS::TaskStats stats[RTOS::MaxTasks];
  rtos.getTaskStats(stats, rtos.count());

  p->printf("OS Tasks:\n");
  p->printf("%12s\t%12s\t%8s\t%12s\n", "Name", "State", "Stack", "Stack used");
  for (uint32_t i = 0; i < rtos.count(); i++) {
    char state[8];
    switch (stats[i].state) {
      case RTOS::Ready:     strcpy(state, "Ready"); break;
      case RTOS::Running:   strcpy(state, "Running"); break;
      case RTOS::Blocked:   strcpy(state, "Blocked"); break;
      case RTOS::Suspended: strcpy(state, "Suspend"); break;
      case RTOS::Deleted:   strcpy(state, "Deleted"); break;
    }
    p->printf("%12s\t%12s\t%8d\t%12d\n", stats[i].name, state, stats[i].stack,
                                       stats[i].stackUsed);
  }
  p->printf("Clock frequency at %dMHz\n", MAP_SysCtlClockGet()/1000000);
  p->printf("Uptime of %d seconds\n",
    (uint32_t)Timer::getTicks64(Timer::Timer0)/MAP_SysCtlClockGet());
  return true;
}

bool CLI::save() {
  char err[32];
  if (!ParameterSave::save(parameters, err)) {
    p->printf("Parameter save error: %s\n", err);
    return false;
  }
  p->putLine("Saved parameter set");
  return true;
}

bool CLI::load() {
  char err[ERR_BYTES];
  if (!ParameterSave::load(parameters, err)) {
    p->printf("Parameter load error: %s\n", err);
    return false;
  }
  p->putLine("Loaded parameter set");
  return true;
}

bool CLI::erase() {
  char err[ERR_BYTES];
  if (!ParameterSave::erase(err)) {
    p->printf("Parameter erase error: %s\n", err);
    return false;
  }
  p->putLine("Erased parameter set");
  return true;
}


// On enter
void CLI::execute() {
  p->putLine("");
  next = 0;

  const char* first = strtok(buf,  " ,\n,\r");

  // Arcane C++ syntax to call a member function pointer
  uint32_t i;
  for (i = 0; i < ARRAY_LENGTH(commandTable); i++) {
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
  if (i == ARRAY_LENGTH(commandTable)) {
    p->printf("\"%s\" is not a command\n", first);
  }
  putPrompt();
}

void CLI::putPrompt() {
  p->put('>');
  p->put(' ');
}
bool CLI::parseInt(const char* const input, uint32_t* result,
                   bool* isNegative) {

  uint32_t num = 0;
  bool hex = false;
  const char* iterator = input;

  if (*iterator == '0' && (*(iterator+1) == 'x' || *(iterator+1) == 'X')) {
    hex = true;
    iterator += 2;
  } else if (*iterator == '-') {
    iterator++;
    if (!isNegative)
      *isNegative = true;
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
