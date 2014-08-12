#ifndef __PRINTER_HPP__
#define __PRINTER_HPP__

#include <stdint.h>
#include <cstdarg>

extern "C" {
  extern void uartISR();
  extern void uartTask(void *params);
}

class Printer {
  public:
    Printer();
    // Only required function for a printer is a put function
    virtual bool put(char c, uint32_t timeout_ms = 0) const = 0;
    void printf(const char* fmt, ...);
    void test();

  private:
    void printDecimal(uint32_t val, bool isSigned) const;
    void printHex(uint32_t val, bool upperCase, bool leader) const;
    void printString(const char* str) const;

    // Flags and state for printf
    bool percent;
    bool hexLeader;
    int pad;
    bool zeros;

};

#endif
