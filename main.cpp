/*
 * Eyal Cohen - Apollo10 main file
 */

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "FreeRTOS.h"
#include "task.h"

#include "rtos.hpp"
#include "serialport.hpp"
#include "cli.hpp"
#include "parameters.hpp"

/* Globals */
SerialPort serialPort;
static RTOS rtos;
static Parameters parameters;
static CLI cli(&serialPort, &parameters, rtos);

int32_t test1, test2, test3;

void ledTask(void *params) {

  enum { Red = GPIO_PIN_1, Blue = GPIO_PIN_2, Green = GPIO_PIN_3,
         All = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 };
  uint32_t freq = 500;

  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, All);

  TickType_t xLastWakeTime = xTaskGetTickCount();

  MAP_GPIOPinWrite(GPIO_PORTF_BASE, All, 0);

  while (1) {

    static bool heartBeat = false;
    //MAP_GPIOPinWrite(GPIO_PORTF_BASE, Red | Blue, heartBeat ? Red | Blue : 0);
    heartBeat = !heartBeat;
    vTaskDelayUntil( &xLastWakeTime, freq );
  }
}

/* Interrupt Service Routines */
void uartISR() {
  serialPort.isr();
}


/* FreeRTOS tasks */

enum Priorities {
  UARTPriority = 1,
  CLIPriority = 1
};

enum Stacks {
  UARTStack = 100,
  CLIStack = 400
};

void uartTask(void *params) {
  serialPort.task(params);
}

void cliTask(void *params) {
  cli.task(params);
}

int main() {

  // Set the clocking to run at 50 MHz from the PLL.
  MAP_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                     SYSCTL_OSC_MAIN);

  rtos.createTask(uartTask, "UartTX", UARTStack, UARTPriority);

  serialPort.initialize(115200);
  serialPort.putLine();
  serialPort.putLine("Apollo10 controller");

  rtos.createTask(cliTask, "CLI", CLIStack, CLIPriority);

  rtos.createTask(ledTask, "LED", 100, 1);

  #define ADD_RO(x, desc) \
    parameters.addParameter(#x, desc, &x, Parameters::ReadOnly);
  #define ADD_RW(x, desc) \
    parameters.addParameter(#x, desc, &x, Parameters::Writable);
  #define ADD_ROM(x, desc) \
    parameters.addParameter(#x, desc, &x, Parameters::FlashWritable);

  ADD_RO(serialPort.rxCount, "Rx Bytes transmitted");
  ADD_ROM(test1, "Test1");
  ADD_RO(serialPort.txCount, "Tx Bytes transmitted");
  ADD_ROM(test2, "Test2");
  ADD_ROM(test3, "Test3");


  vTaskStartScheduler();
  while (1) {}
}
