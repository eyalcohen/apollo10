#include "parameters.hpp"

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

void Parameters::ResultsIterator::operator++() {
  seek();
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
    if (strstr(params->table[next].name, str)) {
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

/*
Parameters::ParameterGet Parameters::get(ParameterIndex index) {
  ParameterGet p = { index,
                     params->table[index].name,
                     params->table[index].description,
                     params->table[index].type,
                     data };

  iter->initialize(this, name);
}
*/

template <typename T>
bool Parameters::set(uint8_t index, T val, char* const error) {
  if (index >= length) {
    if (error) strcpy(error, "Index exceeds parameter length");
    return false;
  }
  if (table[index].qualifier == ReadOnly) {
    if (error) strcpy(error, "This parameter is read-only");
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
  template bool Parameters::set<T>(uint8_t, T, char* const error)
S(int8_t); S(int16_t); S(int32_t);
S(uint8_t); S(uint16_t); S(uint32_t); S(float);
#undef S

