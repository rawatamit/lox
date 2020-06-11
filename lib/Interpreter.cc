#include "Interpreter.h"
#include "lox/LoxCallable.h"
#include "lox/LoxClass.h"
#include "lox/LoxInstance.h"
#include "lox/LoxFunction.h"
#include "lox/LoxReturn.h"
#include "lox/LoxDouble.h"
#include "lox/LoxString.h"
#include "lox/LoxNil.h"
#include "lox/LoxBool.h"
#include "NativeFns.h"

#include <memory>
#include <iostream>

void Interpreter::resolve(std::shared_ptr<Expr> expr, int depth)
{
  locals[expr] = depth;
}

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt>> stmts)
{
  for (auto stmt : stmts)
  {
    execute(stmt);
  }
}

void Interpreter::installNativeFns()
{
  globals->define("clock", std::make_shared<ClockFn>());
}

void Interpreter::checkNumberOperand(Token tok, std::shared_ptr<LoxObject> expr)
{
  if (expr->getType() != LoxObject::DOUBLE)
  {
    throw RuntimeException(tok, "Unary operand must be a number.");
  }
}

void Interpreter::checkNumberOperand(Token tok, std::shared_ptr<LoxObject> exp0, std::shared_ptr<LoxObject> exp1)
{
  if (exp0->getType() != LoxObject::DOUBLE
    or exp1->getType() != LoxObject::DOUBLE)
  {
    throw RuntimeException(tok, "Binary operands must both be numbers.");
  }
}

std::shared_ptr<LoxObject> Interpreter::lookupVariable(const Token& name, std::shared_ptr<Expr> expr)
{
  auto it = locals.find(expr);

  if (it != locals.end())
  {
    return env->get(it->second, name);
  }
  else
  {
    return globals->get(name);
  }
}

std::shared_ptr<LoxObject> Interpreter::execute(std::shared_ptr<Stmt> stmt)
{
  return stmt->accept(*this);
}

std::shared_ptr<LoxObject> Interpreter::executeBlock(
  std::vector<std::shared_ptr<Stmt>>& stmts,
  std::shared_ptr<Environment> newenv)
{
  auto oldenv = env;
  env = newenv;

  try
  {
    for (auto st : stmts)
    {
      execute(st);
    }
  }
  catch(LoxReturn& e)
  {
    // restore the environment
    // in case we execute a return stmt
    env = oldenv;
    throw;
  }
  
  // restore the environment
  env = oldenv;
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::evaluate(std::shared_ptr<Expr> expr)
{
  return expr->accept(*this);
}

std::shared_ptr<LoxObject> Interpreter::visitClass(std::shared_ptr<Class> klass)
{
  std::shared_ptr<LoxObject> superclass = nullptr;

  if (klass->superclass != nullptr)
  {
    superclass = evaluate(klass->superclass);
    if (superclass->getType() != LoxObject::CLASS)
    {
      throw RuntimeException(klass->superclass->name,
       "Superclass must be a class.");
    }
  }

  env->define(klass->name.lexeme, nullptr);

  if (klass->superclass != nullptr)
  {
    env = std::make_shared<Environment>(env);
    env->define("super", superclass);
  }

  std::map<std::string, std::shared_ptr<LoxObject>> methods;
  for (std::shared_ptr<Function> method : klass->methods)
  {
    auto fn =
      std::make_shared<LoxFunction>(method, env,
        method->name.lexeme == "init");
    methods[method->name.lexeme] = fn;
  }

  auto loxklass =
    std::make_shared<LoxClass>(
      klass->name.lexeme,
      std::static_pointer_cast<LoxClass>(superclass),
      methods);
  
  if (superclass != nullptr)
  {
    env = env->getEnclosing();
  }

  env->assign(klass->name, loxklass);
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitFunction(std::shared_ptr<Function> stmt)
{
  auto fn = std::make_shared<LoxFunction>(stmt, env, false);
  env->define(stmt->name.lexeme, fn);
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitExpression(std::shared_ptr<Expression> stmt)
{
  evaluate(stmt->expr);
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitIf(std::shared_ptr<If> stmt)
{
  if (evaluate(stmt->condition)->isTruthy())
  {
    execute(stmt->thenBranch);
  }
  else if (stmt->elseBranch != nullptr)
  {
    execute(stmt->elseBranch);
  }
  
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitPrint(std::shared_ptr<Print> stmt)
{
  auto e = evaluate(stmt->expr);
  std::cout << e->str() << '\n';
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitWhile(std::shared_ptr<While> stmt)
{
  while (evaluate(stmt->condition)->isTruthy())
  {
    execute(stmt->body);
  }
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitReturn(std::shared_ptr<Return> stmt)
{
  std::shared_ptr<LoxObject> value =
    std::make_shared<LoxNil>();
  if (stmt->value != nullptr)
  {
    value = evaluate(stmt->value);
  }

  throw LoxReturn(value);
}

std::shared_ptr<LoxObject> Interpreter::visitVar(std::shared_ptr<Var> stmt)
{
  std::shared_ptr<LoxObject> value =
    std::make_shared<LoxNil>();
  if (stmt->init != nullptr)
  {
    value = evaluate(stmt->init);
  }

  env->define(stmt->name.lexeme, value);
  return nullptr;
}

std::shared_ptr<LoxObject> Interpreter::visitBlock(std::shared_ptr<Block> stmt)
{
  return executeBlock(stmt->stmts, std::make_shared<Environment>(env));
}

std::shared_ptr<LoxObject> Interpreter::visitLogical(std::shared_ptr<Logical> expr)
{
  std::shared_ptr<LoxObject> l = evaluate(expr->left);

  switch (expr->Operator.type)
  {
  case TokenType::OR:
    return l->isTruthy() ? l : evaluate(expr->right);
  case TokenType::AND:
    return ! l->isTruthy() ? l : evaluate(expr->right);
  default:
    throw RuntimeException(expr->Operator, "unknown operator in logical");
  }
}

std::shared_ptr<LoxObject> Interpreter::visitAssign(std::shared_ptr<Assign> expr)
{
  std::shared_ptr<LoxObject> v = evaluate(expr->value);
  auto it = locals.find(std::static_pointer_cast<Expr>(expr));

  if (it != locals.end())
  {
    env->assign(it->second, expr->name, v);
  }
  else
  {
    globals->assign(expr->name, v);
  }
  
  return v;
}

std::shared_ptr<LoxObject> Interpreter::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr)
{
  std::shared_ptr<LoxObject> lhs = evaluate(expr->left);
  std::shared_ptr<LoxObject> rhs = evaluate(expr->right);

  switch (expr->Operator.type)
  {
  case TokenType::MINUS:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxDouble>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      - std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::STAR:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxDouble>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      * std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::SLASH:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxDouble>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      / std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::PLUS:
    if (lhs->getType() == LoxObject::DOUBLE
      and rhs->getType() == LoxObject::DOUBLE)
    {
      return std::make_shared<LoxDouble>(
        std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
        + std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
    }
    else if (lhs->getType() == LoxObject::STRING
        and rhs->getType() == LoxObject::STRING)
    {
      return std::make_shared<LoxString>(
        std::dynamic_pointer_cast<LoxString>(lhs)->getValue()
        + std::dynamic_pointer_cast<LoxString>(rhs)->getValue());
    }

    throw RuntimeException(expr->Operator,
      "Binary operands must be two numbers or two strings.");
  case TokenType::GREATER:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxBool>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      > std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::GREATER_EQUAL:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxBool>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      >= std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::LESS:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxBool>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      < std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::LESS_EQUAL:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::make_shared<LoxBool>(
      std::dynamic_pointer_cast<LoxDouble>(lhs)->getValue()
      <= std::dynamic_pointer_cast<LoxDouble>(rhs)->getValue());
  case TokenType::BANG_EQUAL:
    return std::make_shared<LoxBool>(! lhs->isEqual(rhs));
  case TokenType::EQUAL_EQUAL:
    return std::make_shared<LoxBool>(lhs->isEqual(rhs));
  default:
    return nullptr;
  }
}

std::shared_ptr<LoxObject> Interpreter::visitCall(std::shared_ptr<Call> expr)
{
  std::shared_ptr<LoxObject> callee = evaluate(expr->callee);
  std::vector<std::shared_ptr<LoxObject>> args;

  for (auto arg : expr->args)
  {
    args.push_back(evaluate(arg));
  }

  if (std::shared_ptr<LoxCallable> fn =
        std::dynamic_pointer_cast<LoxCallable>(callee))
  {
    if (args.size() == fn->arity())
    {
      return fn->call(this, args);
    }
    else
    {
      throw RuntimeException(expr->paren,
        "Expected " + std::to_string(fn->arity()) +
        " arguments but got " + std::to_string(args.size())
        + ".");
    }
  }
  else
  {
    throw RuntimeException(expr->paren, "Can only call functions and classes.");
  }
}

std::shared_ptr<LoxObject> Interpreter::visitGet(std::shared_ptr<Get> expr)
{
  std::shared_ptr<LoxObject> object = evaluate(expr->object);
//std::cout << "get " << object.type().name() << '\n';
  if (object->getType() == LoxObject::INSTANCE)
  {
    return std::static_pointer_cast<LoxInstance>(object)->
            get(expr->name);
  }

  throw RuntimeException(expr->name,
    "Only instances have properties.");
}

std::shared_ptr<LoxObject> Interpreter::visitSet(std::shared_ptr<Set> expr)
{
  std::shared_ptr<LoxObject> object = evaluate(expr->object);

  if (object->getType() == LoxObject::INSTANCE)
  {
    std::shared_ptr<LoxObject> v = evaluate(expr->value);
    std::static_pointer_cast<LoxInstance>(object)->
      set(expr->name, v);
    return v;
  }

  throw RuntimeException(expr->name,
    "Only instances have fields.");
}

std::shared_ptr<LoxObject> Interpreter::visitThis(std::shared_ptr<This> expr)
{
  return lookupVariable(expr->keyword,
    std::dynamic_pointer_cast<Expr>(expr));
}

std::shared_ptr<LoxObject> Interpreter::visitSuper(std::shared_ptr<Super> expr)
{
  int distance = locals.find(expr)->second;

  std::shared_ptr<LoxClass> superclass =
    std::static_pointer_cast<LoxClass>(
      env->get(distance, "super"));

  std::shared_ptr<LoxInstance> object =
    std::static_pointer_cast<LoxInstance>(
      env->get(distance - 1, "this"));
  
  std::shared_ptr<LoxFunction> method =
    std::static_pointer_cast<LoxFunction>(
      superclass->findMethod(expr->method.lexeme));
  
  if (method == nullptr)
  {
    throw RuntimeException(expr->method,                          
        "Undefined property '" + expr->method.lexeme + "'.");     
  }

  return method->bind(object);
}

std::shared_ptr<LoxObject> Interpreter::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr)
{
  return evaluate(expr->expression);
}

std::shared_ptr<LoxObject> Interpreter::visitLiteralExpr(std::shared_ptr<LiteralExpr> expr)
{
  switch (expr->type)
  {
  case TokenType::NUMBER:
    return std::make_shared<LoxDouble>(strtod(expr->value.c_str(), NULL));
  case TokenType::NIL:
    return std::make_shared<LoxNil>();
  case TokenType::STRING:
    return std::make_shared<LoxString>(expr->value);
  case TokenType::TRUE:
    return std::make_shared<LoxBool>(true);
  case TokenType::FALSE:
    return std::make_shared<LoxBool>(false);
  default:
    throw RuntimeException("unknown type visitLiteralExpr()"); 
  }
}

std::shared_ptr<LoxObject> Interpreter::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr)
{
  std::shared_ptr<LoxObject> val = evaluate(expr->right);

  switch (expr->Operator.type)
  {
  case TokenType::MINUS:
    checkNumberOperand(expr->Operator, val);
    return std::make_shared<LoxDouble>(
      - std::static_pointer_cast<LoxDouble>(val)->getValue());
  
  case TokenType::BANG:
    return std::make_shared<LoxBool>(!val->isTruthy());
  
  default:
    return nullptr;
  }
}

std::shared_ptr<LoxObject> Interpreter::visitVariable(std::shared_ptr<Variable> expr)
{
  return lookupVariable(expr->name,
    std::static_pointer_cast<Expr>(expr));
}
