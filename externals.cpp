
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

void __error__(char *pcFilename, uint32_t ui32Line) {
  while (1) {
  }
}

void __cxa_pure_virtual() {

}
}

