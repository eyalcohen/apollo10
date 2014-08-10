#include "FreeRTOS.h"
#include "task.h"

void abort(void) {
  while (1) {}
}


void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName) {
  while (1) {}
}

void __error__(char *pcFilename, uint32_t ui32Line) {
  while (1) {
  }
}

