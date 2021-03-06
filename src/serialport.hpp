#ifndef __SERIALPORT_HPP__
#define __SERIALPORT_HPP__

#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "printer.hpp"

extern "C" {
  extern void uartISR();
  extern void uartTask(void *params);
}

class SerialPort : public Printer {
  public:
    SerialPort();
    void initialize(uint32_t baud);

    // These get and set from the FreeRTOS queues
    virtual bool put(char c, uint32_t timeout_ms=20) const;
    char get(uint32_t timeout_ms=0) const;

    void putLine(const char* str = "", uint32_t timeout_ms = 20,
                 bool lineEnd = true) const;
    // Interrupt service routine for receiving characters on the serial port
    void isr();

    // FreeRTOS task for transmitting characters
    void task(void *params);

    // serial port counters
    uint16_t rxCount;
    uint16_t txCount;

  private:
    enum {BufferSize = 128};
    QueueHandle_t rxQueue, txQueue;
};

#endif
