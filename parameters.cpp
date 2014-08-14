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

