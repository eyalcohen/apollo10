/*
 * uart.cpp - Serial communication across a uart
 * Eyal Cohen
 *
 */

#include "serialport.hpp"

#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "task.h"

extern SerialPort serialPort;

void uartISR() {
  serialPort.isr();
}

void uartTask(void *params) {
  serialPort.task(params);
}

SerialPort::SerialPort() {

}

void SerialPort::initialize(uint32_t baud) {

    rxQueue = xQueueCreate(BufferSize, sizeof(uint8_t));
    txQueue = xQueueCreate(BufferSize, sizeof(uint8_t));

    // Enable peripherals
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins properly
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    MAP_UARTConfigSetExpClk(UART0_BASE, 16000000, baud,
                            (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_WLEN_8));


    // Enable 8 character deep FIFO
    MAP_UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    // Interrupt management
    MAP_UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(INT_UART0);
}

bool SerialPort::put(uint8_t c, uint32_t timeout_ms) {
  bool success = true;
  if (timeout_ms > 0) {
    success = xQueueSendToBack(txQueue, &c, timeout_ms);
  } else {
    BaseType_t requestContextSwitch = false;
    xQueueSendToBackFromISR(txQueue, &c, &requestContextSwitch);
  }
  return success;
}

uint8_t SerialPort::get(uint32_t timeout_ms) const {
  char c;
  bool result = xQueueReceive(rxQueue, &c, timeout_ms);
  if (!result)
    return -1;
  else return c;
}

void SerialPort::putLine(const char* str, uint32_t timeout_ms, bool lineEnd) {
  for (int i = 0; str[i] != '\0'; i++) {
    serialPort.put(str[i], timeout_ms);
  }
  if (lineEnd) {
    serialPort.put('\r');
    serialPort.put('\n');
  }
}

void SerialPort::isr() {
  BaseType_t requestContextSwitch = false;
  BaseType_t requestContextSwitch2 = false;
  do {
    uint8_t c = MAP_UARTCharGetNonBlocking(UART0_BASE);
    xQueueSendToBackFromISR(rxQueue, &c, &requestContextSwitch);
    xQueueSendToBackFromISR(txQueue, &c, &requestContextSwitch2);
  } while (UARTCharsAvail(UART0_BASE));

  if (requestContextSwitch) {
    taskYIELD();
  }
}

void SerialPort::task(void *params) {
  while (1) {
    char c = -1;
    // Block on getting a character from the queue
    bool result = xQueueReceive(txQueue, &c, portMAX_DELAY);
    // Send out a character, this blocks!
    if (c != -1 && result)
      MAP_UARTCharPut(UART0_BASE, c);
  }
}