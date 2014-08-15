#include "../printer.hpp"
#include <stdio.h>

class PrinterTest : public Printer{
  virtual bool put(char c, uint32_t timeout_ms = 0) const {
    putchar(c);
  }
} printer;

int main() {
  printer.printf("Test1\n");
  printer.printf("Test2 %d %d %8d %08d\n", 52384, -52384, 52384, 52384);
  printer.printf("Test3 %i %i %8i %08i\n", 52384, -52384, 52384, 52384);
  printer.printf("Test4 %u %u %8u %08u\n", 52384, -52384, 52384, 52384);
  printer.printf("Test5 %x %x %8x %08x\n", 52384, -52384, 52384, 52384);
  printer.printf("Test6 %X %#X %8X %08X\n", 52384, -52384, 52384, 52384);
  printer.printf("Test7 %d %4d %04d %04d\n", -5, -5, -5, -5);
  printer.printf("Test8 %s\n", "Eyal");
}
