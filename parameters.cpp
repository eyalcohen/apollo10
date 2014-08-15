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


void Parameters::addParameter(const char* name,
                              const char* description,
                              void* data, Type type,
                              Qualifier qualifier) {

  table[length].name = name;
  table[length].description = description;
  table[length].data = data;
  table[length].qualifier = qualifier;
  table[length].type = type;
  length++;
}

void Parameters::get(const char* name, ResultsIterator* iter) {
  iter->initialize(this, name);
}

template <typename T>
bool Parameters::set(uint8_t index, T val, char* const error) {
  if (index >= length) {
    if (error) strcpy(error, "Index exceeds parameter length");
    return false;
  }
  if (table[length].qualifier == ReadOnly) {
    if (error) strcpy(error, "This parameter is read-only");
    return false;
  }
  if (!val) {
    if (error) strcpy(error, "null pointer");
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

template bool Parameters::set<int8_t>(uint8_t, int8_t, char* const error);
template bool Parameters::set<int16_t>(uint8_t, int16_t, char* const error);
template bool Parameters::set<int32_t>(uint8_t, int32_t, char* const error);
template bool Parameters::set<uint8_t>(uint8_t, uint8_t, char* const error);
template bool Parameters::set<uint16_t>(uint8_t, uint16_t, char* const error);
template bool Parameters::set<uint32_t>(uint8_t, uint32_t, char* const error);
template bool Parameters::set<float>(uint8_t, float, char* const error);
