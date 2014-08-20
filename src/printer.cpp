/*
 * printer.cpp - Provides some printing functions like printf
 * Author: Eyal Cohen
 *
 */

#include "printer.hpp"

#include <stdbool.h>

Printer::Printer() : percent(false), hexLeader(false), pad(0), zeros(false) {}

void Printer::printDecimal(uint32_t val, bool isSigned) const {
  char out[16] = {'\0'};
  char* buf = out;
  int32_t asSigned = (int32_t)val;

  // val is the actual value we print, so invert asSigned and throw in a
  // negative sign
  bool neg = isSigned && asSigned < 0;
  if (neg) {
    val = -asSigned;
  }

  // Prepare for output buffer
  do {
    uint32_t digit = val % 10;
    *buf++ = digit + '0';
    val /= 10;
  } while (val && (buf - out) != sizeof(out));

  // Pad if necessary, adding a negative sign either before or after the padding
  // depenidng on whether 0's are requested

  if (neg && !zeros)
    *buf++ = '-';
  for (int add = pad - (buf - out) - (neg ? 1 : 0); add > 0; add--) {
    *buf++ = zeros ? '0' : ' ';
  }
  if (neg && zeros)
    *buf++ = '-';

  // now reverse and print
  for (;buf != out;buf--) {
    put(*buf);
  }
  put(*buf);
}

void Printer::printHex(uint32_t val, bool upperCase, bool leader) const {
  char out[16] = {'\0'};
  char* buf = out;

  // Put hex leader 0x or 0X if requested
  if (leader) {
    put('0');
    put(upperCase ? 'X' : 'x');
  }

  // Prepare for output buffer
  do {
    uint32_t hex = val % 16;
    char adder = upperCase ? 'A' : 'a';
    *buf++ = (hex >= 10) ? hex - 10 + adder : hex + '0';
    val /= 16;
  } while (val && (buf - out) != sizeof(out));

  // Pad if necessary
  for (int add = pad - (buf - out); add > 0; add--) {
    *buf++ = zeros ? '0' : ' ';
  }

  // now reverse and print
  for (;buf != out;buf--) {
    put(*buf);
  }
  put(*buf);
}

void Printer::printString(const char* str) const {
  int16_t count = 0;
  for (;*str;str++) {
    count++;
  }
  str -= count;
  count = pad - count;
  while (count > 0) {
    put(' ');
    count--;
  }
  for (;*str;str++) {
    put(*str);
  }
}

void Printer::printf(const char* fmt, ...) {
  va_list args;
  va_start (args, fmt);
  for (;*fmt;fmt++) {
    // Begin format parsing
    if (*fmt == '%') {
      percent = true;
      hexLeader = false;
      pad = 0;
      zeros = false;
    } else if (percent) {

      // handle padding
      if (*fmt >= '0' && *fmt <= '9') {
        if (*fmt == '0' && pad == 0)
          zeros = true;
        else {
          pad = (pad * 10) + (*fmt - '0');
        }
      }

      switch (*fmt) {
        case 'd':
        case 'i':
        case 'u':
          printDecimal(va_arg(args, uint32_t), *fmt != 'u');
          percent = false;
          break;
        case 'x':
        case 'X':
          printHex(va_arg(args, uint32_t), *fmt == 'X', hexLeader);
          percent = false;
          break;
        case 's':
          printString(va_arg(args, const char*));
          percent = false;
          break;
        case 'c':
          put(va_arg(args, int));
          percent = false;
          break;
        case '#':
          hexLeader = true;
          break;
      }
    } else {
      put(*fmt);
    }
  }
}
