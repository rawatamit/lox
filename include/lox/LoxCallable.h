#ifndef _LOX_CALLABLE_H_
#define _LOX_CALLABLE_H_

#include "Interpreter.h"
#include "lox/LoxObject.h"
#include <memory>
#include <vector>

namespace lox {

class LoxCallable : public LoxObject {
public:
  LoxCallable(LoxObject::ObjT type) : LoxObject(type) {}

  virtual ~LoxCallable() = default;

  virtual std::shared_ptr<LoxObject>
  call(Interpreter *interpreter,
       std::vector<std::shared_ptr<LoxObject>> &args) = 0;
  virtual unsigned arity() const = 0;
  virtual std::string str() const { return "<native_fn>"; }
};

} // namespace lox

#endif
