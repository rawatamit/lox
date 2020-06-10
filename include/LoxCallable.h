#ifndef _LOX_CALLABLE_H_
#define _LOX_CALLABLE_H_

#include "Interpreter.h"
#include <any>
#include <vector>

namespace lox
{

class LoxCallable
{
public:
  virtual std::any call(Interpreter* interpreter, std::vector<std::any>& args) = 0;
  virtual unsigned arity() const = 0;
  virtual std::string str() const
  { return "<native_fn>"; }
};

} // namespace

#endif
