#ifndef _LOX_RETURN_H_
#define _LOX_RETURN_H_

#include <any>
#include <stdexcept>

namespace lox
{

class LoxReturn : std::runtime_error
{
private:
  std::any value;

public:
  LoxReturn(std::any value) :
    std::runtime_error("LoxReturn"), value(value)
  {}

  std::any getValue() const
  { return value; }
};

} // namespace

#endif
