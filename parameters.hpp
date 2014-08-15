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

    typedef uint8_t ParameterIndex;

    // Result from a get request
    struct ParameterGet {
      ParameterIndex index;
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
        ParameterIndex next;
        const Parameters* params;
        const char* str;
    };

    template <typename T>
    void addParameter(const char* name, const char* description,
                      T* data, Qualifier qualifier);
    void get(const char* name, ResultsIterator* iter);
    /*
    ParameterGet get(ParameterIndex index);
    */

    // val is converted to the type in the table.  Returns true if succesful
    template <typename T>
    bool set(ParameterIndex index, T val, char* const error = NULL);

  private:

    struct ParameterEntry {
      const char* name;
      const char* description;
      Qualifier qualifier;
      void* data; // Type information is stored as an enum
      Type type;
    };

    // This macro defines many setType functions that can be used to set the
    // parameter type.  Its a clever way to avoid having to send type
    // information to addParameter, akin to templating, with compile time
    // information only
    #define S(T1, T2) void setType(ParameterIndex idx, T1 t) { table[idx].type = T2; }
    S(uint32_t, Uint32)
    S(uint16_t, Uint16)
    S(uint8_t, Uint8)
    S(int8_t, Int8)
    S(int16_t, Int16)
    S(int32_t, Int32)
    S(float, Float)
    #undef S

    enum {MaxParams = 32};
    ParameterIndex length;
    ParameterEntry table[MaxParams];

};

#endif