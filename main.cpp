/*
 * Eyal Cohen - Apollo10 main file
 */

#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "FreeRTOS.h"
#include "task.h"

#include "priorities.h"
#include "serialport.hpp"

SerialPort serialPort;

int main() {

    // Set the clocking to run at 50 MHz from the PLL.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    serialPort.initialize(115200);
    serialPort.putLine("Initializing");

    xTaskCreate(uartTask, "UartTX", SERIAL_PORT_STACK,
                   NULL, tskIDLE_PRIORITY + SERIAL_PORT_PRIORITY, NULL);

    vTaskStartScheduler();
    while (1) {}
}
