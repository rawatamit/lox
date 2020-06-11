#ifndef _LOX_FUNCTION_H_
#define _LOX_FUNCTION_H_

#include "Stmt.h"
#include "LoxCallable.h"
#include "LoxNil.h"
#include "LoxReturn.h"
#include "Environment.h"
#include <memory>

namespace lox
{

class LoxInstance;

class LoxFunction : public LoxCallable
{
private:
  std::shared_ptr<Function> declaration;
  std::shared_ptr<Environment> closure;
  bool isInitializer;

public:
  LoxFunction(
      std::shared_ptr<Function> declaration,
      std::shared_ptr<Environment> closure,
      bool isInitializer) :
    LoxCallable(LoxObject::FUNCTION),
    declaration(declaration),
    closure(closure),
    isInitializer(isInitializer)
  {}

  virtual ~LoxFunction() = default;

  virtual std::shared_ptr<LoxObject> call(Interpreter* interpreter, std::vector<std::shared_ptr<LoxObject>>& args) override 
  {
    auto env = std::make_shared<Environment>(closure);

    for (std::vector<Token>::size_type i = 0; i < declaration->params.size(); ++i)
    {
      env->define(declaration->params[i].lexeme, args[i]);
    }

    try
    {
      interpreter->executeBlock(declaration->body, env);
    }
    catch (const LoxReturn& e)
    {
      if (isInitializer)
      {
        return closure->get(0, "this");
      }
      else
      {
        return e.getValue();
      }
    }
    
    if (isInitializer)
    {
      return closure->get(0, "this");
    }
    else
    {
      return std::make_shared<LoxNil>();
    }
  }

  virtual unsigned arity() const override
  {
    return declaration->params.size();
  }

  std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance)
  {
    auto env = std::make_shared<Environment>(closure);
    env->define("this", std::static_pointer_cast<LoxObject>(instance));
    return std::make_shared<LoxFunction>(declaration, env, isInitializer);
  }

  virtual std::string str() const override
  { return "<fn " + declaration->name.lexeme + ">"; }

  virtual bool isEqual(std::shared_ptr<LoxObject> arg0) const override
  {
    if (arg0 == nullptr || arg0->getType() != LoxObject::FUNCTION)
    {
      return false;
    }
    else
    {
      return std::static_pointer_cast<LoxFunction>(arg0).get() == this;
    }
  }
};

} // namespace

#endif
