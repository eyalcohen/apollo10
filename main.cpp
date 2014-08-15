/*
 * Eyal Cohen - Apollo10 main file
 */

#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "FreeRTOS.h"
#include "task.h"

#include "priorities.h"
#include "serialport.hpp"
#include "cli.hpp"
#include "parameters.hpp"

//CLI cli;

void ledTask(void *params) {

  #define RED_GPIO_PIN_CFG        GPIO_PF1_T0CCP1
  #define BLUE_GPIO_PIN_CFG       GPIO_PF2_T1CCP0
  #define GREEN_GPIO_PIN_CFG      GPIO_PF3_T1CCP1
  #define RED_GPIO_PIN            GPIO_PIN_1
  #define BLUE_GPIO_PIN           GPIO_PIN_2
  #define GREEN_GPIO_PIN          GPIO_PIN_3

/*
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  MAP_GPIOPinConfigure(GREEN_GPIO_PIN_CFG);
  MAP_GPIOPinConfigure(RED_GPIO_PIN_CFG);
  MAP_GPIOPinConfigure(BLUE_GPIO_PIN_CFG);
  MAP_GPIOPadConfigSet(SYSCTL_PERIPH_GPIOF, GREEN_GPIO_PIN, GPIO_STRENGTH_8MA_SC,
                       GPIO_PIN_TYPE_STD);

  //MAP_GPIOPinWrite(SYSCTL_PERIPH_GPIOF, GREEN_GPIO_PIN, true);
*/

  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1000;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //serialPort.test();

  }
}

/* Globals */
SerialPort serialPort;
Parameters parameters;
CLI cli(&serialPort, &parameters);

int32_t test = 0;

/* Interrupt Service Routines */
void uartISR() {
  serialPort.isr();
}

/* FreeRTOS tasks */
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

  xTaskCreate(uartTask, "UartTX", SERIAL_PORT_STACK,
                 NULL, tskIDLE_PRIORITY + SERIAL_PORT_PRIORITY, NULL);

  serialPort.initialize(115200);
  serialPort.putLine();
  serialPort.putLine("Apollo10 controller");

  xTaskCreate(cliTask, "CLI", CLI_PORT_STACK,
                 NULL, tskIDLE_PRIORITY + CLI_PORT_PRIORITY, NULL);

  #define ADD_RO(x, desc) \
    parameters.addParameter(#x, desc, &x, Parameters::ReadOnly);
  #define ADD_RW(x, desc) \
    parameters.addParameter(#x, desc, &x, Parameters::Writable);

  ADD_RO(serialPort.rxCount, "Rx Bytes transmitted");
  ADD_RO(serialPort.txCount, "Tx Bytes transmitted");
  ADD_RW(test, "test");
  /*
    parameters.addParameter("test", "bytes transmitted",
                 (void*)&test, Parameters::Int32,
                 Parameters::Writable);
                 */

  /*
  xTaskCreate(ledTask, "LED", LED_PORT_STACK,
                 NULL, tskIDLE_PRIORITY + LED_PORT_PRIORITY, NULL);
                 */

  vTaskStartScheduler();
  while (1) {}
}
