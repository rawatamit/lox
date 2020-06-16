#ifndef _LOXINSTANCE_H_
#define _LOXINSTANCE_H_

#include <any>
#include <map>
#include <string>

#include "LoxClass.h"
#include "LoxFunction.h"
#include "LoxObject.h"
#include "RuntimeException.h"

namespace lox {

class LoxInstance : public LoxObject,
                    public std::enable_shared_from_this<LoxInstance> {
private:
  LoxClass *klass;
  std::map<std::string, std::shared_ptr<LoxObject>> fields;

public:
  LoxInstance(LoxClass *klass) : LoxObject(LoxObject::INSTANCE), klass(klass) {}

  ~LoxInstance() = default;

  std::shared_ptr<LoxObject> get(const Token &name) {
    auto field = fields.find(name.lexeme);
    if (field != fields.end()) {
      return field->second;
    }

    if (auto method = klass->findMethod(name)) {
      auto fn = std::dynamic_pointer_cast<LoxFunction>(method);
      return fn->bind(shared_from_this());
    }

    throw RuntimeException(name, "Undefined property '" + name.lexeme + "'.");
  }

  void set(const Token &name, std::shared_ptr<LoxObject> v) {
    fields[name.lexeme] = v;
  }

  virtual std::string str() const override {
    return klass->getName() + " instance";
  }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override {
    if (arg0 == nullptr || arg0->getType() != LoxObject::INSTANCE) {
      return false;
    } else {
      return std::static_pointer_cast<LoxInstance>(arg0).get() == this;
    }
  }
};

} // namespace lox

#endif
