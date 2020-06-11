#ifndef _LOXCLASS_H_
#define _LOXCLASS_H_

#include "LoxCallable.h"

#include <map>
#include <string>
#include <memory>

namespace lox
{

class LoxClass : public LoxCallable
{
private:
  std::string name;
  std::map<std::string, std::any> methods;

public:
  LoxClass(const std::string& name,
           std::map<std::string, std::any> methods);
  virtual std::any call(Interpreter*, std::vector<std::any>&) override;
  virtual unsigned arity() const override;
  std::any findMethod(const std::string& name) const;
  std::any findMethod(const Token& name) const;
  virtual std::string str() const override;
};

} // namespace lox

#endif
