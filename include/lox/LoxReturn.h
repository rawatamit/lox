#ifndef _LOX_RETURN_H_
#define _LOX_RETURN_H_

#include <any>
#include <stdexcept>

namespace lox
{

class LoxReturn : std::runtime_error
{
private:
  std::shared_ptr<LoxObject> value;

public:
  LoxReturn(std::shared_ptr<LoxObject> value) :
    std::runtime_error("LoxReturn"), value(value)
  {}

  std::shared_ptr<LoxObject> getValue() const
  { return value; }
};

} // namespace

#endif
