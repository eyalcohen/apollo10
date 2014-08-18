#include "parameters.hpp"
#include <string.h>

Parameters::ResultsIterator::ResultsIterator() :
  next(0), params(NULL), str(NULL) {}

void Parameters::ResultsIterator::initialize(const Parameters* params,
                                             const char* name) {
  this->str = name;
  this->params = params;
}

void Parameters::ResultsIterator::reset() {
  next = 0;
  seek();
}

Parameters::ResultsIterator& Parameters::ResultsIterator::operator++() {
  seek();
  return *this;
}

Parameters::ParameterGet Parameters::ResultsIterator::val() {
  if (!params) {
    return ParameterGet();
  }
  uint32_t data;
  switch (params->table[next].type) {
    case Uint8:  data = *(uint8_t*) params->table[next].data; break;
    case Uint16: data = *(uint16_t*)params->table[next].data; break;
    default:
    case Uint32: data = *(uint32_t*)params->table[next].data; break;
    case Int8:   data = *(int8_t*)  params->table[next].data; break;
    case Int16:  data = *(int16_t*) params->table[next].data; break;
    case Int32:  data = *(int32_t*) params->table[next].data; break;
  }

  ParameterGet p = { next,
                     params->table[next].name,
                     params->table[next].description,
                     params->table[next].type,
                     data };
  next++;
  return p;
}

bool Parameters::ResultsIterator::complete() {
  return next != params->length;
}

void Parameters::ResultsIterator::seek() {
  if (!params || !str)
    return;

  for (;next < params->length; next++) {
    const char* result = strstr(params->table[next].name, str);
    if (result == params->table[next].name || !strlen(str)) {
      break;
    }
  }
}


template <typename T>
void Parameters::addParameter(const char* name,
                              const char* description,
                              T* data, 
                              Qualifier qualifier) {

  table[length].name = name;
  table[length].description = description;
  table[length].data = (void*)data;
  table[length].qualifier = qualifier;
  setType(length, *data);
  length++;
}

// Define the only template specializations we will end up using
#define S(T) \
  template void Parameters::addParameter<T> \
    (const char*, const char*, T*, Qualifier)
S(int8_t); S(int16_t); S(int32_t);
S(uint8_t); S(uint16_t); S(uint32_t); S(float);
#undef S

void Parameters::get(const char* name, ResultsIterator* iter) {
  iter->initialize(this, name);
}

bool Parameters::get(ParameterIndex index, ParameterGet* get) {

  if (!get) return false;
  if (index > MaxParams) return false;

  uint32_t data;
  switch (table[index].type) {
    case Uint8:  data = *(uint8_t*) table[index].data; break;
    case Uint16: data = *(uint16_t*)table[index].data; break;
    default:
    case Uint32: data = *(uint32_t*)table[index].data; break;
    case Int8:   data = *(int8_t*)  table[index].data; break;
    case Int16:  data = *(int16_t*) table[index].data; break;
    case Int32:  data = *(int32_t*) table[index].data; break;
  }

  get->index = index;
  get->name = table[index].name;
  get->description = table[index].description;
  get->type = table[index].type;
  get->valueUntyped = data;

  return true;
}

template <typename T>
bool Parameters::set(ParameterIndex index, T val, char err[ERR_BYTES]) {
  if (index >= length) {
    if (err) strncpy(err, "Index exceeds parameter length", ERR_BYTES);
    return false;
  }
  if (table[index].qualifier == ReadOnly) {
    if (err) strncpy(err, "This parameter is read-only", ERR_BYTES);
    return false;
  }

  switch (table[index].type) {
    case Int8:   *(int8_t*)   table[index].data = val; break;
    case Int16:  *(int16_t*)  table[index].data = val; break;
    case Int32:  *(int32_t*)  table[index].data = val; break;
    case Uint8:  *(uint8_t*)  table[index].data = val; break;
    case Uint16: *(uint16_t*) table[index].data = val; break;
    case Uint32: *(uint32_t*) table[index].data = val; break;
    case Float:  *(float*)    table[index].data = val; break;
  }

  return true;
}

template <typename T>
bool Parameters::set(const char* name, T val, char err[ERR_BYTES]) {
  ParameterIndex index = MaxParams;
  for (ParameterIndex i = 0; i < length; i++) {
    if (strncmp(name, table[i].name, MaxParamName) == 0) {
      index = i;
      break;
    }
  }

  if (index == MaxParams) {
    if (err) strncpy(err, "Could not find this parameter", ERR_BYTES);
    return false;
  }

  if (table[index].qualifier == ReadOnly) {
    if (err) strncpy(err, "This parameter is read-only", ERR_BYTES);
    return false;
  }

  switch (table[index].type) {
    case Int8:   *(int8_t*)   table[index].data = val; break;
    case Int16:  *(int16_t*)  table[index].data = val; break;
    case Int32:  *(int32_t*)  table[index].data = val; break;
    case Uint8:  *(uint8_t*)  table[index].data = val; break;
    case Uint16: *(uint16_t*) table[index].data = val; break;
    case Uint32: *(uint32_t*) table[index].data = val; break;
    case Float:  *(float*)    table[index].data = val; break;
  }

  return true;
}


#define S(T) \
  template bool Parameters::set<T>(ParameterIndex, T, char err[ERR_BYTES]); \
  template bool Parameters::set<T>(const char*, T, char err[ERR_BYTES])
S(int8_t); S(int16_t); S(int32_t);
S(uint8_t); S(uint16_t); S(uint32_t); S(float);
#undef S

