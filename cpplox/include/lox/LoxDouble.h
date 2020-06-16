#ifndef _LOX_DOUBLE_H_
#define _LOX_DOUBLE_H_

#include "lox/LoxObject.h"
#include <string>

namespace lox {

class LoxDouble : public LoxObject {
private:
  double value;

public:
  LoxDouble(double value) : LoxObject(LoxObject::DOUBLE), value(value) {}

  double getValue() const { return value; }

  virtual std::string str() const override {
    auto res = std::to_string(value);
    auto dot = res.find('.');

    size_t end = res.size() - 1;
    while (end > dot and res[end] == '0') {
      --end;
    }

    // hack for integers
    if (end == dot) {
      --end;
    }
    return res.substr(0, end + 1);
  }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override {
    if (arg0 == nullptr || (arg0->getType() != LoxObject::DOUBLE)) {
      return false;
    }

    // TODO: bad comparsison for doubles
    return value == std::static_pointer_cast<LoxDouble>(arg0)->getValue();
  }
};

} // namespace lox

#endif
