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
    virtual bool put(char c, uint32_t timeout_ms=0) const;
    char get(uint32_t timeout_ms=0) const;

    void putLine(const char* str = "", uint32_t timeout_ms = 0,
                 bool lineEnd = true) const;
    // Interrupt service routine for receiving characters on the serial port
    void isr();

    // FreeRTOS task for transmitting characters
    void task(void *params);


  private:
    enum {BufferSize = 128};
    QueueHandle_t rxQueue, txQueue;
};

#endif
