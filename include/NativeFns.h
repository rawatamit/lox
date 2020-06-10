#ifndef _NATIVE_FNS_H_
#define _NATIVE_FNS_H_

#include "LoxCallable.h"
#include <ctime>

namespace lox
{

class ClockFn : public LoxCallable
{
public:
    std::any call(Interpreter*, std::vector<std::any>&)
    {
      return static_cast<double>(clock());
    }

    unsigned arity() const
    {
      return 0;
    }
};

}

#endif
