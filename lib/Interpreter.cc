#include "Interpreter.h"
#include "LoxCallable.h"
#include "LoxClass.h"
#include "LoxInstance.h"
#include "LoxFunction.h"
#include "LoxReturn.h"
#include "NativeFns.h"
#include "Utils.h"

#include <memory>
#include <iostream>

void Interpreter::resolve(Expr* expr, int depth)
{
  locals[expr] = depth;
}

void Interpreter::interpret(std::vector<Stmt*> stmts)
{
  for (Stmt* stmt : stmts)
  {
    execute(stmt);
  }
}

void Interpreter::installNativeFns()
{
  globals->define("clock", std::make_any<LoxCallable*>(new ClockFn()));
}

void Interpreter::checkNumberOperand(Token tok, std::any expr)
{
  if (expr.type() != typeid(double))
  {
    throw RuntimeException(tok, "Unary operand must be a number.");
  }
}

void Interpreter::checkNumberOperand(Token tok, std::any exp0, std::any exp1)
{
  if (exp0.type() != typeid(double) or
      exp1.type() != typeid(double))
  {
    throw RuntimeException(tok, "Binary operands must both be numbers.");
  }
}

std::any Interpreter::lookupVariable(const Token& name, Expr* expr)
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

std::any Interpreter::execute(Stmt* stmt)
{
  return stmt->accept(this);
}

std::any Interpreter::executeBlock(
  std::vector<Stmt*>& stmts,
  std::shared_ptr<Environment> newenv)
{
  auto oldenv = env;
  env = newenv;

  try
  {
    for (Stmt* st : stmts)
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

std::any Interpreter::evaluate(Expr* expr)
{
  return expr->accept(this);
}

std::any Interpreter::visitClass(Class* klass)
{
  env->define(klass->name.lexeme, nullptr);

  std::map<std::string, std::any> methods;
  for (Function* method : klass->methods)
  {
    auto fn =
      std::make_any<LoxCallable*>(
        new LoxFunction(method, env,
          method->name.lexeme == "this"));
    methods[method->name.lexeme] = fn;
  }

  auto loxklass =
    std::make_any<LoxCallable*>(
      new LoxClass(klass->name.lexeme, methods));
  env->assign(klass->name, loxklass);
  return nullptr;
}

std::any Interpreter::visitFunction(Function* stmt)
{
  auto fn =
    std::make_any<LoxCallable*>(
      new LoxFunction(stmt, env, false));
  env->define(stmt->name.lexeme, fn);
  return nullptr;
}

std::any Interpreter::visitExpression(Expression* stmt)
{
  evaluate(stmt->expr);
  return nullptr;
}

std::any Interpreter::visitIf(If* stmt)
{
  if (isTruthy(evaluate(stmt->condition)))
  {
    execute(stmt->thenBranch);
  }
  else if (stmt->elseBranch != nullptr)
  {
    execute(stmt->elseBranch);
  }
  
  return nullptr;
}

std::any Interpreter::visitPrint(Print* stmt)
{
  auto e = evaluate(stmt->expr);
  std::cout << stringify(e) << '\n';
  return nullptr;
}

std::any Interpreter::visitWhile(While* stmt)
{
  while (isTruthy(evaluate(stmt->condition)))
  {
    execute(stmt->body);
  }
  return nullptr;
}

std::any Interpreter::visitReturn(Return* stmt)
{
  std::any value = nullptr;
  if (stmt->value != nullptr)
  {
    value = evaluate(stmt->value);
  }

  throw LoxReturn(value);
}

std::any Interpreter::visitVar(Var* stmt)
{
  std::any value = nullptr;
  if (stmt->init != nullptr)
  {
    value = evaluate(stmt->init);
  }

  env->define(stmt->name.lexeme, value);
  return nullptr;
}

std::any Interpreter::visitBlock(Block* stmt)
{
  return executeBlock(stmt->stmts, std::make_shared<Environment>(env));
}

std::any Interpreter::visitLogical(Logical* expr)
{
  std::any l = evaluate(expr->left);

  switch (expr->Operator.type)
  {
  case TokenType::OR:
    return isTruthy(l) ? l : evaluate(expr->right);
  case TokenType::AND:
    return ! isTruthy(l) ? l : evaluate(expr->right);
  default:
    throw RuntimeException(expr->Operator, "unknown operator in logical");
  }
}

std::any Interpreter::visitAssign(Assign* expr)
{
  std::any v = evaluate(expr->value);
  auto it = locals.find(expr);

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

std::any Interpreter::visitBinaryExpr(BinaryExpr* expr)
{
  std::any lhs = evaluate(expr->left);
  std::any rhs = evaluate(expr->right);

  switch (expr->Operator.type)
  {
  case TokenType::MINUS:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) - std::any_cast<double>(rhs);
  case TokenType::STAR:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) * std::any_cast<double>(rhs);
  case TokenType::SLASH:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) / std::any_cast<double>(rhs);
  case TokenType::PLUS:
    if (lhs.type() == typeid(double) and rhs.type() == typeid(double))
    {
      return std::any_cast<double>(lhs) + std::any_cast<double>(rhs);
    }
    else if (lhs.type() == typeid(std::string)
         and rhs.type() == typeid(std::string))
    {
      auto l = std::any_cast<std::string>(lhs);
      auto r = std::any_cast<std::string>(rhs);
      std::string res;
      res.append(l);
      res.append(r);
      return res;
    }

    throw RuntimeException(expr->Operator,
      "Binary operands must be two numbers or two strings.");
  case TokenType::GREATER:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) > std::any_cast<double>(rhs);
  case TokenType::GREATER_EQUAL:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) >= std::any_cast<double>(rhs);
  case TokenType::LESS:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) < std::any_cast<double>(rhs);
  case TokenType::LESS_EQUAL:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return std::any_cast<double>(lhs) <= std::any_cast<double>(rhs);
  case TokenType::BANG_EQUAL:
    return ! isEqual(lhs, rhs);
  case TokenType::EQUAL_EQUAL:
    return isEqual(lhs, rhs);
  default:
    return nullptr;
  }
}

std::any Interpreter::visitCall(Call* expr)
{
  std::any callee = evaluate(expr->callee);
  std::vector<std::any> args;

  for (Expr* arg : expr->args)
  {
    args.push_back(evaluate(arg));
  }

  try
  {
    LoxCallable* fn = std::any_cast<LoxCallable*>(callee);

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
  catch (const std::bad_cast&)
  {
    throw RuntimeException(expr->paren, "Can only call functions and classes.");
  }
}

std::any Interpreter::visitGet(Get* expr)
{
  std::any object = evaluate(expr->object);
std::cout << "get " << object.type().name() << '\n';
  if (object.type() == typeid(LoxInstance*))
  {
    return std::any_cast<LoxInstance*>(object)->
            get(expr->name);
  }

  throw RuntimeException(expr->name,
    "Only instances have properties.");
}

std::any Interpreter::visitSet(Set* expr)
{
  std::any object = evaluate(expr->object);

  if (object.type() == typeid(LoxInstance*))
  {
    std::any v = evaluate(expr->value);
    std::any_cast<LoxInstance*>(object)->
      set(expr->name, v);
    return v;
  }

  throw RuntimeException(expr->name,
    "Only instances have fields.");
}

std::any Interpreter::visitThis(This* expr)
{
  return lookupVariable(expr->keyword, expr);
}

std::any Interpreter::visitGroupingExpr(GroupingExpr* expr)
{
  return evaluate(expr->expression);
}

std::any Interpreter::visitLiteralExpr(LiteralExpr* expr)
{
  switch (expr->type)
  {
  case TokenType::NUMBER:
    return strtod(expr->value.c_str(), NULL);
  case TokenType::NIL:
    return nullptr;
  case TokenType::STRING:
    return expr->value;
  case TokenType::TRUE:
    return true;
  case TokenType::FALSE:
    return false;
  default:
    throw RuntimeException("unknown type visitLiteralExpr()"); 
  }
}

std::any Interpreter::visitUnaryExpr(UnaryExpr* expr)
{
  std::any val = evaluate(expr->right);

  switch (expr->Operator.type)
  {
  case TokenType::MINUS:
    checkNumberOperand(expr->Operator, val);
    return - std::any_cast<double>(val);
  
  case TokenType::BANG:
    return ! isTruthy(val);
  
  default:
    return nullptr;
  }
}

std::any Interpreter::visitVariable(Variable* expr)
{
  return lookupVariable(expr->name, expr);
}
