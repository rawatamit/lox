#ifndef _LOX_NIL_H_
#define _LOX_NIL_H_

#include "lox/LoxObject.h"
#include <memory>
#include <string>

namespace lox
{

class LoxNil : public LoxObject
{
public:
  LoxNil() :
    LoxObject(LoxObject::NIL)
  {}

  virtual bool isTruthy() const override
  {
    return false;
  }

  virtual std::string str() const override
  {
      return "nil";
  }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override
  {
    if (arg0 != nullptr)
    {
      return arg0->getType() == LoxObject::NIL;
    }

    return false;
  }
};

} // namespace lox

#endif
