#ifndef __SERIALPORT_HPP__
#define __SERIALPORT_HPP__

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

extern "C" {
  extern void uartISR();
  extern void uartTask(void *params);
}

class SerialPort {
  public:
    SerialPort();
    void initialize(uint32_t baud);

    // These get and set from the FreeRTOS queues
    void put(uint8_t c);
    uint8_t get();

    // Interrupt service routine for receiving characters on the serial port
    void isr();

    // FreeRTOS task for transmitting characters
    void task(void *params);



  private:
    // Gets a character, doesn't block
    enum {BufferSize = 64};
    QueueHandle_t rxQueue, txQueue;
};

#endif
