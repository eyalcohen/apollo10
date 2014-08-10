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
    bool put(uint8_t c, uint32_t timeout_ms=0);
    uint8_t get(uint32_t timeout_ms=0) const;

    void putLine(const char* str, uint32_t timeout_ms = 0, bool lineEnd = true);

    // Interrupt service routine for receiving characters on the serial port
    void isr();

    // FreeRTOS task for transmitting characters
    void task(void *params);


  private:
    enum {BufferSize = 64};
    QueueHandle_t rxQueue, txQueue;
};

#endif
