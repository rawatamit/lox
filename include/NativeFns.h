#ifndef _NATIVE_FNS_H_
#define _NATIVE_FNS_H_

#include "lox/LoxCallable.h"
#include "lox/LoxDouble.h"
#include <ctime>
#include <memory>
#include <string>

namespace lox {

class ClockFn : public LoxCallable {
public:
  ClockFn() : LoxCallable(LoxObject::BUILTIN) {}

  virtual std::shared_ptr<LoxObject>
  call(Interpreter *, std::vector<std::shared_ptr<LoxObject>> &) override {
    return std::make_shared<LoxDouble>(clock());
  }

  virtual unsigned arity() const override { return 0; }

  virtual std::string str() const override { return "<builtin clock>"; }
};

} // namespace lox

#endif
