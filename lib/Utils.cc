#include "Utils.h"
#include "LoxCallable.h"
#include "RuntimeException.h"

bool isTruthy(std::any expr)
{
  if (expr.type() == typeid(std::nullptr_t))
  {
    return false;
  }
  else if (expr.type() == typeid(bool))
  {
    return std::any_cast<bool>(expr);
  }
  else
  {
    return true;
  }
}

bool isEqual(std::any arg0, std::any arg1)
{
  if (arg0.type() == typeid(std::nullptr_t) and
      arg1.type() == typeid(std::nullptr_t))
  {
    return true;
  }
  else if (arg0.type() == typeid(std::nullptr_t) or
            arg1.type() == typeid(std::nullptr_t))
  {
    return false;
  }
  else if (arg0.type() == typeid(double) and
            arg1.type() == typeid(double))
  {
    auto e0 = std::any_cast<double>(arg0);
    auto e1 = std::any_cast<double>(arg1);

    return e0 == e1;
  }
  else if (arg0.type() == typeid(std::string) and
            arg1.type() == typeid(std::string))
  {
    auto e0 = std::any_cast<std::string>(arg0);
    auto e1 = std::any_cast<std::string>(arg1);

    return e0 == e1;
  }
  else if (arg0.type() == typeid(bool) and
            arg1.type() == typeid(bool))
  {
    auto e0 = std::any_cast<bool>(arg0);
    auto e1 = std::any_cast<bool>(arg1);

    return e0 == e1;
  }

  return false;
}

std::string stringify(std::any r)
{
  if (r.type() == typeid(std::nullptr_t))
  {
    return "nil";
  }
  else if (r.type() == typeid(bool))
  {
    return std::any_cast<bool>(r) ? "true" : "false";
  }
  else if (r.type() == typeid(double))
  {
    auto res = std::to_string(std::any_cast<double>(r));
    auto dot = res.find('.');

    size_t end = res.size() - 1;
    while (end > dot and res[end] == '0')
    {
      --end;
    }

    // hack for integers
    if (end == dot) { --end; }
    return res.substr(0, end + 1);
  }
  else if (r.type() == typeid(std::string))
  {
    return std::any_cast<std::string>(r);
  }
  else if (r.type() == typeid(LoxCallable*))
  {
    return std::any_cast<LoxCallable*>(r)->str();
  }
  else
  {
    throw RuntimeException("stringify()");
  }
}
