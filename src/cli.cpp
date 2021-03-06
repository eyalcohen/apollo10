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

// get [num|name]
// If num, we try parsing the number as an int than calling the appropriate
// parameter function.  If name, we expect an iterator with all the possible
// results
bool CLI::getParameter() {
  const char* nextArg = strtok(NULL, " ,\n,\r");
  Parameters::ResultsIterator it;
  Parameters::ParameterGet param;
  uint32_t paramNumber = (uint32_t)-1;

  // First try parsing as if the first argument was number
  if (nextArg && parseInt(nextArg, &paramNumber)) {
    if (!parameters->get(paramNumber, &param))
      return false;
  // Otherwise, try parsing as if it were a string
  } else if (nextArg) {
    parameters->get(nextArg, &it);
  // If no argument is provided, get the whole paramete rlist
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

void CLI::printParam(const Parameters::ParameterGet* param) const {
  p->printf("[%02d] %s = ", param->index, param->name, param->description);
  if (param->type == Parameters::Int8) {
    p->printf("%d", *(int8_t*)&param->valueUntyped);
  } else if (param->type == Parameters::Int16) {
    p->printf("%d", *(int16_t*)&param->valueUntyped);
  } else if (param->type == Parameters::Int32) {
    p->printf("%d", *(int32_t*)&param->valueUntyped);
  } else if (param->type == Parameters::Float) {
    p->printf("%f", *(float*)&param->valueUntyped);
  } else {
    p->printf("%u", param->valueUntyped);
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

  float valAsFloat;
  bool valIsFloat = false;
  if (parseFloat(valueString, &valAsFloat)) {
    valIsFloat = true;
  }
  else if (!parseInt(valueString, &val, &isNegative)) {
    p->printf("\"%s\" is not a value\n", valueString);
    return false;
  }

  char err[32];
  bool success = false;
  if (paramIsNum) {

    if (valIsFloat) {
      success = parameters->set(param, valAsFloat, err);
    } else {
      success = parameters->set(param, isNegative ? -((int32_t)val): (uint32_t)val, err);
    }
    if (!success) {
      p->printf("Error %s\n", err);
      return false;
    }

    // Print parameter from integer
    Parameters::ParameterGet paramGet;
    parameters->get(param, &paramGet);
    printParam(&paramGet);

  } else {

    if (valIsFloat) {
      success = parameters->set(param, valAsFloat, err);
    } else {
      success = parameters->set(param, isNegative ? -((int32_t)val): (uint32_t)val, err);
    }
    if (!success) {
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

void CLI::putPrompt() const {
  p->put('>');
  p->put(' ');
}

bool CLI::parseFloat(const char* input, float* result) const {
  uint32_t num = 0;
  const char* it = input;
  bool neg = false;

  if (*it == '-') {
    neg = true;
    it++;
  }

  for(; *it && *it != '.'; it++) {
    num *= 10;
    if (*it >= '0' && *it <= '9') {
      num += *it - '0';
    } else {
      return false;
    }
  }

  uint32_t frac = 0;
  uint16_t exp = 0;

  if (*it++ != '.') {
    return false;
  }

  for(; *it && *it != '.'; it++) {
    frac *= 10;
    if (*it >= '0' && *it <= '9') {
      frac += *it - '0';
      exp++;
    } else {
      return false;
    }
  }

  // having a number like 3. isn't valid
  if (exp < 1)
    return false;

  float divideBy = 1.0;
  while (exp--) {
    divideBy = divideBy*10.0;
  }

  float res = num + frac/divideBy;
  if (neg) res = -res;
  *result = res;
  return true;

}

bool CLI::parseInt(const char* input, uint32_t* result,
                   bool* isNegative) const {

  uint32_t num = 0;
  bool hex = false;
  const char* it = input;

  if (*it == '0' && (*(it+1) == 'x' || *(it+1) == 'X')) {
    hex = true;
    it += 2;
  } else if (*it == '-') {
    it++;
    if (!isNegative)
      *isNegative = true;
  }

  for(; *it; it++) {
    num *= hex ? 16 : 10;
    if (hex && *it >= 'a' && *it <= 'f') {
      num += (*it - 'a' + 10);
    } else if (hex && *it >= 'A' && *it <= 'F') {
      num += (*it - 'A' + 10);
    } else if (*it >= '0' && *it <= '9'){
      num += *it - '0';
    } else {
      return false;
    }
  }

  *result = num;
  return true;

}
