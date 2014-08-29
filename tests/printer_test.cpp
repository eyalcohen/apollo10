#include "printer.hpp"
#include <stdio.h>

class PrinterTest : public Printer{
  virtual bool put(char c, uint32_t timeout_ms = 0) const {
    putchar(c);
    fflush(stdout);
  }
} printer;


#define PRINTF(a, b, c, d, e) \
  num++; \
  printer.printf("Test   #%d: " a "\n", num, b, c, d, e); \
  printf("Actual #%d: " a "\n\n", num, b, c, d, e);

int main() {

  uint16_t num = 0;

  PRINTF("%d %d %d %d", 1, 2, 3, 4);
  PRINTF("%d %d %8d %08d", 52384, -52384, 52384, 52384);
  PRINTF("%i %i %8i %08i", 52384, -52384, 52384, 52384);
  PRINTF("%u %u %8u %08u", 52384, -52384, 52384, 52384);
  PRINTF("%x %x %8x %08x", 52384, -52384, 52384, 52384);
  PRINTF("%X %#X %8X %08X", 52384, -52384, 52384, 52384);
  PRINTF("%d %3d %04d %04d", -5, -5, -5, 5);
  PRINTF("%d %3d %04d %04d", -50, -50, -50, 50);
  PRINTF("%s %s %s %s", "Eyal", "Cohen", "Says", "Hello");
  PRINTF("%f %f %f %f", 6234.567689, -1234.56789, .01234, -.01234);
  PRINTF("%f %f %f %f", 125.0, -125.0, .125, .125);
  PRINTF("%.8f %.8f %.8f %.8f", 1234.567689, -1234.56789, .01234, -.01234);
  PRINTF("%.4f %.4f %.4f %.4f", 1.234567689, -1.23456789, .1234, -.1234);
  PRINTF("%8.4f %8.4f %8.4f %8.4f", 1.234567689, -1.23456789, .1234, -.1234);
  PRINTF("%08.4f %08.4f %08.4f %08.4f", 1.234567689, -1.23456789, .1234, -.1234);
  PRINTF("%08.8f %08.8f %08.8f %08.8f", 1.234567689, -1.23456789, .1234, -.1234);
  PRINTF("%016.16f %016.16f %16.16f %16.16f", 1.234567689, -1.23456789, .1234, -.1234);
}
