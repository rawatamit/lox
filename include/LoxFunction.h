#ifndef _LOX_FUNCTION_H_
#define _LOX_FUNCTION_H_

#include "Stmt.h"
#include "LoxCallable.h"
#include "LoxReturn.h"
#include "Environment.h"

namespace lox
{

class LoxInstance;

class LoxFunction : public LoxCallable
{
private:
  Function* declaration;
  std::shared_ptr<Environment> closure;
  bool isInitializer;

public:
  LoxFunction(
      Function* declaration,
      std::shared_ptr<Environment> closure,
      bool isInitializer) :
    declaration(declaration),
    closure(closure),
    isInitializer(isInitializer)
  {}

  virtual std::any call(Interpreter* interpreter, std::vector<std::any>& args) override 
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
    return nullptr;
  }

  virtual unsigned arity() const override
  {
    return declaration->params.size();
  }

  LoxFunction* bind(LoxInstance* instance)
  {
    auto env = std::make_shared<Environment>(closure);
    env->define("this", instance);
    return new LoxFunction(declaration, env, isInitializer);
  }

  virtual std::string str() const override
  { return "<fn " + declaration->name.lexeme + ">"; }
};

} // namespace

#endif
