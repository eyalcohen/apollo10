#include "parameter_save.hpp"

#include <stdint.h>
#include <string.h>

#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

// TODO: Make crc an actual crc

ParameterSave::ParameterSave() {

}


bool ParameterSave::init(char err[ERR_BYTES]) {
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  if (EEPROMInit() == EEPROM_INIT_ERROR) {
    if (err) { strncpy(err, "Failed to inialize EEPROM", ERR_BYTES); }
    return false; 
  }
  return true;
}


bool ParameterSave::save(const Parameters* parameters, 
                         char err[ERR_BYTES]) {

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

bool ParameterSave::load(Parameters* parameters, char err[ERR_BYTES]) {

  if (EEPROMStatusGet()) {
    if (err) { strncpy(err, "EEPROM status fail", ERR_BYTES); }
    return false;
  }
  Header header;

  uint32_t headerSize = align32(sizeof(header));
  EEPROMRead((uint32_t*)&header, 0, headerSize);
  uint32_t address = headerSize;

  // Check header for correctness
  if (header.version != ParameterVersion) {
    if (err) { strncpy(err, "Parameter version mismatch", ERR_BYTES); }
    return false;
  }
  // FIXME
  if (header.crc32 != 0xECECECEC) {
    if (err) { strncpy(err, "CRC mismatch", ERR_BYTES); }
    return false;
  }

  // Load all parameters from EEPROM
  for (uint32_t i = 0; i < header.parameterCount; i++) {
    // Get Param information
    ParamHeader paramHeader;
    uint32_t paramHeaderSize = align32(sizeof(paramHeader));
    EEPROMRead((uint32_t*)&paramHeader, address, paramHeaderSize);
    address += paramHeaderSize;
    char name[64];
    EEPROMRead((uint32_t*)name, address, MIN(paramHeader.nameLen, sizeof(name)));
    address += paramHeader.nameLen;

    // Now copy to parameters
    for (uint32_t i = 0; i < parameters->length; i++) {
      if (strncmp(parameters->table[i].name, name, sizeof(name)) == 0) {
        if (parameters->table[i].qualifier == Parameters::FlashWritable
            && parameters->table[i].type == paramHeader.type) {
          void* data = parameters->table[i].data;
          switch (paramHeader.type) {
            case Parameters::Bool:   *(bool*) data = paramHeader.value; break;
            case Parameters::Uint8:  *(uint8_t*) data = paramHeader.value; break;
            case Parameters::Uint16: *(uint16_t*)data = paramHeader.value; break;
            case Parameters::Uint32: *(uint32_t*)data = paramHeader.value; break;
            case Parameters::Int8:   *(int8_t*)  data = paramHeader.value; break;
            case Parameters::Int16:  *(int16_t*) data = paramHeader.value; break;
            case Parameters::Int32:  *(int32_t*) data = paramHeader.value; break;
            case Parameters::Float:  *(float*)   data = paramHeader.value; break;
          }
          break;
        }
      }
    }

  }

  return true;
}

bool ParameterSave::erase(char err[ERR_BYTES]) {
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
