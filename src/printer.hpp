/*
 * printer.hpp
 * Author: Eyal Cohen
 *
 * Provides some printing functions like printf.  This
 * class is intended to be subclassed from, and requires that the
 * derived class implements a put character function
 *
 * Supports %d, %i, %u, %s, %x, %X
 * Suports #, width, padding and zeros.  eg %#08X
 */

#ifndef __PRINTER_HPP__
#define __PRINTER_HPP__

#include <stdint.h>
#include <cstdarg>

class Printer {
  public:
    Printer();

    // This function is required and puts a character somewhere
    virtual bool put(char c, uint32_t timeout_ms = 20) const = 0;

    // Like C printf
    void printf(const char* fmt, ...);

  private:
    void printDecimal(uint32_t val, bool isSigned) const;
    void printHex(uint32_t val, bool upperCase, bool leader) const;
    void printString(const char* str) const;
    void printFloat(double asDouble) const;

    // Flags and state for printf
    bool percent; // % symbol
    bool hexLeader; // # symbol, like %#0X
    int8_t pad; // Pad modifier, like %8X
    bool zeros; // Add zeros to padding
    int8_t precision; // Precision flag

    enum {DefaultPrecision = 6};
};

#endif
