#ifndef _LOX_STRING_H_
#define _LOX_STRING_H_

#include "lox/LoxObject.h"
#include <string>

namespace lox
{

class LoxString : public LoxObject
{
private:
  std::string value;

public:
  LoxString(const std::string& value) :
    LoxObject(LoxObject::STRING),
    value(value)
  {}

  std::string getValue() const
  {
    return value;
  }

  virtual std::string str() const override
  {
      return value;
  }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override
  {
    if (arg0 == nullptr || (arg0->getType() != LoxObject::STRING))
    {
      return false;
    }

    return value == std::static_pointer_cast<LoxString>(arg0)->getValue();
  }
};

} // namespace lox

#endif
