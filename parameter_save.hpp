/*
 * parameter_save.hpp
 * Author: Eyal Cohen
 *
 * Stores ROM parameters in EEPROM.  Note!  Single-threaded, if this
 * needs to be called from interrupts or tasks for any reason (and why
 * would it?) then add some OS exclusion guards.
 */

#ifndef __PARAMETER_SAVE_HPP__
#define __PARAMETER_SAVE_HPP__

#include <stdint.h>
#include <string.h>
#include  "driverlib/eeprom.h"
#include "common.hpp"

#include "serialport.hpp"
extern SerialPort serialPort;

class ParameterSave {
  public:
    ParameterSave();

    // Returns false if init failed
    static bool init(char err[ERR_BYTES]) {
      MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
      if (EEPROMInit() == EEPROM_INIT_ERROR) {
        if (err) { strncpy(err, "Failed to inialize EEPROM", ERR_BYTES); }
        return false; 
      }
      return true;
    }

    static bool save(const Parameters* parameters, char err[ERR_BYTES]) {

      // big assignment on the stack
      Parameters::ParameterIndex count = 0;

      Header header;
      uint32_t address = 0;
      address += sizeof(header);
      align32(address);

      /* Pseudocode
       * Get size of header
       * Set counter to header size in bytes
       * For every FlashWritable parameter
       *   Count the size of the parameter name
       *   Program into EEPROM the parameter header (name length, type, value)
       *   Program into EEPROM the variable-length parameter name
       *   Increment the EEPROM pointer by sizeof parameterHeader + parameterNameCount
       */

      for (uint32_t i = 0; i < parameters->length; i++) {
        if (parameters->table[i].qualifier == Parameters::FlashWritable) {
          ParamHeader paramHeader;
          paramHeader.nameLen = align32(strlen(parameters->table[i].name));
          paramHeader.value = *(uint32_t*)parameters->table[i].data;
          paramHeader.type = parameters->table[i].type;

          if (EEPROMStatusGet()) {
            if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
            return false;
          }

          uint32_t len = align32(sizeof(paramHeader)); 
          if (EEPROMProgram((uint32_t*)&paramHeader, address, len)) {
            if (err) { strncpy(err, "EEPROM program fail", ERR_BYTES); }
            return false;
          }

          address += len;

          if (EEPROMStatusGet()) {
            if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
            return false;
          }

          if (EEPROMProgram((uint32_t*)(parameters->table[i].name), address, 
                            paramHeader.nameLen)) {
            if (err) { strncpy(err, "EEPROM program fail", ERR_BYTES); }
            return false;
          }

          address += paramHeader.nameLen;
          count++;

        }
      }

      header.parameterCount = count;
      header.crc32 = 0xECECECEC;
      header.version = ParameterVersion;

      if (EEPROMStatusGet()) {
        if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
        return false;
      }

      if (count == 0) {
        if (err) { strncpy(err, "Nothing to do", ERR_BYTES); }
        return false;
      }

      uint32_t headerSize = align32(sizeof(header));
      if (EEPROMProgram((uint32_t*)&header, 0, headerSize)) {
        if (err) { strncpy(err, "EEPROM program fail", ERR_BYTES); }
        return false;
      }

      if (EEPROMStatusGet()) {
        if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
        return false;
      }

      return true;
    }

    static bool load(Parameters* parameters, char err[ERR_BYTES]) {
      if (EEPROMStatusGet()) {
        if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
        return false;
      }
      Header header;

      uint32_t headerSize = align32(sizeof(header));
      // Length must be multiples of 4
      EEPROMRead((uint32_t*)&header, 0, headerSize);
      uint32_t address = headerSize;

      // Debug
      serialPort.printf("Header version %d count %d crc %8x\n", header.version,
                header.parameterCount, header.crc32);

      if (header.version != ParameterVersion) {
        if (err) { strncpy(err, "Parameter version mismatch", ERR_BYTES); }
        return false;
      }
      // FIXME
      if (header.crc32 != 0xECECECEC) {
        if (err) { strncpy(err, "CRC mismatch", ERR_BYTES); }
        return false;
      }

      for (uint32_t i = 0; i < header.parameterCount; i++) {
        ParamHeader paramHeader;
        uint32_t paramHeaderSize = align32(sizeof(paramHeader));
        EEPROMRead((uint32_t*)&paramHeader, address, paramHeaderSize);
        address += paramHeaderSize;
        char name[64];
        EEPROMRead((uint32_t*)name, address, paramHeader.nameLen);
        serialPort.printf("Param name %s val %d type %d\n", name, paramHeader.value, paramHeader.type);
        address += paramHeader.nameLen;
      }

      return true;
    }

    static bool erase(char err[ERR_BYTES]) {
      if (EEPROMStatusGet()) {
        if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
        return false;
      }
      if (EEPROMMassErase()) {
        return false;
        if (err) { strncpy(err, "EEPROM erase fail", ERR_BYTES); }
      }
      return true;
    }

  private:
    struct Header {
      Parameters::ParameterIndex parameterCount;
      uint32_t crc32;
      uint8_t version;
    };

    struct ParamHeader {
      uint8_t nameLen;
      uint32_t value;
      Parameters::Type type;
    };

    enum { ParameterVersion = 0 };
    enum { ParamsCanSave = 32 };

    // round a number to the nearest 4th
    static uint32_t align32(uint32_t v) {
      uint32_t r = v % 4;  
      return r ? v + (4 - r) : v;
    }
};

#endif
