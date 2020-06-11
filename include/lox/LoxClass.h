#ifndef _LOXCLASS_H_
#define _LOXCLASS_H_

#include "LoxCallable.h"

#include <map>
#include <memory>
#include <string>

namespace lox {

class LoxClass : public LoxCallable {
private:
  std::string name;
  std::shared_ptr<LoxClass> superclass;
  std::map<std::string, std::shared_ptr<LoxObject>> methods;

public:
  LoxClass(const std::string &name, std::shared_ptr<LoxClass> superclass,
           std::map<std::string, std::shared_ptr<LoxObject>> methods);
  virtual std::shared_ptr<LoxObject>
  call(Interpreter *, std::vector<std::shared_ptr<LoxObject>> &) override;
  virtual unsigned arity() const override;
  std::string getName() const;
  std::shared_ptr<LoxObject> findMethod(const std::string &name) const;
  std::shared_ptr<LoxObject> findMethod(const Token &name) const;
  virtual std::string str() const override;
  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override;
};

} // namespace lox

#endif
