#ifndef _LOX_BOOL_H_
#define _LOX_BOOL_H_

#include "lox/LoxObject.h"
#include <string>

namespace lox
{

class LoxBool : public LoxObject
{
private:
  bool value;

public:
  LoxBool(bool value) :
    LoxObject(LoxObject::BOOL),
    value(value)
  {}

  bool getValue() const
  {
    return value;
  }

  virtual bool isTruthy() const override
  {
    return value;
  }

  virtual std::string str() const override
  {
      return (value == true) ? "true" : "false";
  }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override
  {
    if (arg0 == nullptr || (arg0->getType() != LoxObject::BOOL))
    {
      return false;
    }

    return value == std::static_pointer_cast<LoxBool>(arg0)->getValue();
  }
};

} // namespace lox

#endif
