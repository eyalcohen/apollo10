
#include "FreeRTOS.h"
#include "task.h"

#include "serialport.hpp"

extern SerialPort serialPort;

extern "C" {
void abort(void) {
  while (1) {}
}


void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName) {
  serialPort.printf("Stack overflow\n");
  while (1) {}
}

void __error__(char *file, uint32_t line) {
  serialPort.printf("Driverlib assert error in %s, line %d", file, line);
}

void __cxa_pure_virtual() {

}
}

