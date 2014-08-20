/*
 * parameters.hpp
 * Author: Eyal Cohen
 *
 * A table of parameters that are stored in RAM.  Parameters
 * are pointers to 8, 16 or 32-bit values
 */

#ifndef __PARAMETERS_HPP__
#define __PARAMETERS_HPP__

#include <stdint.h>

#include "common.hpp"

class Parameters {

  friend class ParameterSave;

  public:
    Parameters() : length(0) {}

    // FlashWritable parametes can be stored in a ROM.
    enum Qualifier {
      ReadOnly, Writable, FlashWritable
    };

    // Supported types
    enum Type {
      Bool, Int8, Int16, Int32, Uint8, Uint16, Uint32, Float
    };

    // typedeffed if we ever need more than 256 indexes
    typedef uint8_t ParameterIndex;

    // Result from a get request
    struct ParameterGet {
      ParameterIndex index;
      const char* name;
      const char* description;
      Type type;
      uint32_t valueUntyped;
    };

    // Used to iterate over results from get requests.  A client
    // may make a get request and get this iterator as a result.
    class ResultsIterator {
      public:
        ResultsIterator();
        void initialize(const Parameters* params, const char* name);
        void reset();
        ResultsIterator& operator++();
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

    bool get(ParameterIndex index, ParameterGet* get);

    // val is converted to the type in the table.  Returns true if succesful
    template <typename T>
    bool set(ParameterIndex index, T val, char err[ERR_BYTES]);

    template <typename T>
    bool set(const char* name, T val, char err[ERR_BYTES]);

  private:

    // The actual stored entry
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
    #define S(T1, T2) void setType(ParameterIndex idx, T1 t) { table[idx].type = T2; } \

    S(bool, Bool)
    S(uint32_t, Uint32)
    S(uint16_t, Uint16)
    S(uint8_t, Uint8)
    S(int8_t, Int8)
    S(int16_t, Int16)
    S(int32_t, Int32)
    S(float, Float)
    #undef S

    // Max parameters we currently support
    enum {MaxParams = 32};

    // Max parameters we currently support
    enum {MaxParamName = 64};

    // Current number of parmaeters
    ParameterIndex length;

    // The actual parameter table
    ParameterEntry table[MaxParams];

};

#endif
