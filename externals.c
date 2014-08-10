#include "FreeRTOS.h"
#include "task.h"

void abort(void) {
  while (1) {}
}


void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName) {
  while (1) {}
}

