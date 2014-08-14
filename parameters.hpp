/*
 * parameters.hpp
 * Author: Eyal Cohen
 *
 *
 */

#ifndef __PARAMETERS_HPP__
#define __PARAMETERS_HPP__

#include <stdint.h>
#include <string.h>

class Parameters {
  public:
    Parameters() : length(0) {}

    enum Qualifier {
      ReadOnly, Writable
    };

    enum Type {
      Int8, Int16, Int32, Uint8, Uint16, Uint32, Float
    };

    // Result from a get request
    struct ParameterGet {
      uint16_t index;
      const char* name;
      const char* description;
      Type type;
      uint32_t valueUntyped;
    };

    // Used to iterate over results from get requests
    class ResultsIterator {
      public:
        ResultsIterator();
        void initialize(const Parameters* params, const char* name);
        void reset();
        void operator++();
        ParameterGet val();
        bool complete();

      private:
        void seek();
        uint16_t next;
        const Parameters* params;
        const char* str;
    };

    void addParameter(const char* name, const char* description,
                      void* data, Type type, Qualifier qualifier);
    void get(const char* name, ResultsIterator* iter);

  private:

    struct ParameterEntry {
      const char* name;
      const char* description;
      Qualifier qualifier;
      void* data; // Type information is stored as an enum
      Type type;
    };

    enum {MaxParams = 32};
    uint8_t length;
    ParameterEntry table[MaxParams];

};

#endif
