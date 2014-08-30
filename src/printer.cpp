/*
 * printer.cpp - Provides some printing functions like printf
 * Author: Eyal Cohen
 *
 */

#include "printer.hpp"

#include <stdbool.h>
#include <float.h>

Printer::Printer() : percent(false), hexLeader(false), pad(0), zeros(false),
                     precision(-1) {}

void Printer::printDecimal(uint32_t val, bool isSigned) const {
  char out[32];
  char* buf = out;
  int32_t asSigned = (int32_t)val;

  // val is the actual value we print, so invert asSigned and throw in a
  // neg sign
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

  // Pad if necessary, adding a neg sign either before or after the padding
  // depenidng on whether 0's are requested

  int16_t add = pad - (neg ? 1 : 0) - (buf - out);

  if (neg && !zeros)
    *buf++ = '-';

  // determinehow much padding to add
  // remove 1 for the negative sign, and 1 for each character we intend to print
  for (; add > 0; add--) {
    *buf++ = zeros ? '0' : ' ';
  }
  if (neg && zeros)
    *buf++ = '-';

  // now reverse and print
  for (buf--;buf > out;buf--) {
    put(*buf);
  }
  put(*buf);
}

void Printer::printHex(uint32_t val, bool upperCase, bool leader) const {
  char out[32];
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
  for (buf--;buf > out;buf--) {
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

void Printer::printFloat(double asDouble) const {

  float val = asDouble;
  int16_t exponent = 0;
  uint16_t precision_ = precision == -1 ? DefaultPrecision : precision;


  bool neg = val < 0;
  if (neg) val = -val;

  if (val > DBL_MAX) {
    if (neg) {
      put('-');
    }
    for (int16_t add = pad - 3 - (neg ? 1 : 0); add > 0; add--) {
      put(' ');
    }
    put('i');
    put('n');
    put('f');
    return;
  }

  while (val >= 10) {
    exponent++;
    val /= 10;
  }
  while (val < 1 && val !=0) {
    exponent--;
    val *= 10;
  }

  if (neg && zeros) {
    put('-');
  }

  // determinehow much padding to add
  // 1 for negative sign, 1 for decimal, precision, exponent, and 1 for leading
  // number
  int16_t add = pad - (neg ? 1 : 0) - (precision_ ? 1 : 0) - precision_ -
                (exponent > 0 ? exponent : 0) - 1;

  // add padding
  while (add > 0) {
    put(zeros ? '0' : ' ');
    add--;
  }

  if (neg && !zeros) {
    put('-');
  }

  // leading 0
  if (exponent < 0) {
    put('0');
  }

  // Prepare for output buffer
  for (; exponent >= 0; exponent--) {
    // Use a cast to truncate;
    uint32_t digit = val;
    put(digit + '0');
    val = (val - digit) * 10;
  }

  if (precision_ > 0) {
    put('.');
  }

  for (; precision_ > 0; precision_--) {
    if (exponent < -1) {
      put('0');
      exponent++;
    } else {
      uint32_t digit = val;
      put(digit + '0');
      val = (val - digit) * 10;
    }
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
      precision = -1;
    } else if (percent) {

      // handle padding
      if (*fmt >= '0' && *fmt <= '9') {
        if (precision >= 0) {
          precision = (precision * 10) + (*fmt - '0');
        }
        else {
          if (*fmt == '0' && pad == 0)
            zeros = true;
          else {
            pad = (pad * 10) + (*fmt - '0');
          }
        }
      }

      switch (*fmt) {
        case 'f':
          printFloat(va_arg(args, double));
          percent = false;
          break;
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
        case '.':
          precision = 0;
          break;
      }
    } else {
      put(*fmt);
    }
  }
}

