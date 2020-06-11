#ifndef _LOX_OBJECT_H_
#define _LOX_OBJECT_H_

#include <memory>
#include <string>

namespace lox {

class LoxObject {
public:
  enum ObjT {
    DOUBLE,
    STRING,
    NIL,
    BOOL,
    CLASS,
    FUNCTION,
    INSTANCE,
    RETURN,
    BUILTIN,
  };

private:
  ObjT type;

public:
  LoxObject(ObjT type) : type(type) {}

  virtual ~LoxObject() = default;

  ObjT getType() const { return type; }

  virtual bool isTruthy() const { return true; }

  virtual std::string str() const { return "<LoxObject>"; }

  virtual bool isEqual(std::shared_ptr<LoxObject>) const { return false; }
};

} // namespace lox

#endif
