#include "Interpreter.h"
#include <iostream>

void Interpreter::interpret(std::vector<Stmt*> stmts)
{
  try
  {
    for (Stmt* stmt : stmts)
    {
      execute(stmt);
    }
  }
  catch (const RuntimeException& e)
  {
    std::cerr << "[line " << e.tok.line << "]: " << e.what() << '\n';
  }
}

std::any Interpreter::execute(Stmt* stmt)
{
  return stmt->accept(this);
}

std::any Interpreter::evaluate(Expr* expr)
{
  return expr->accept(this);
}

std::any Interpreter::visitExpression(Expression* stmt)
{
  evaluate(stmt->expr);
  return nullptr;
}

std::any Interpreter::visitPrint(Print* stmt)
{
  auto e = evaluate(stmt->expr);
  std::cout << stringify(e) << '\n';
  return nullptr;
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
  auto cenv = env;
  env = std::make_shared<Environment>(cenv);

  for (Stmt* st : stmt->stmts)
  {
    execute(st);
  }

  env = cenv;
  return nullptr;
}

std::any Interpreter::visitAssign(Assign* expr)
{
  std::any v = evaluate(expr->value);
  env->assign(expr->name, v);
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
      auto l = std::any_cast<double>(lhs);
      auto r = std::any_cast<double>(rhs);

      return l + r;
    }
    else if (lhs.type() == typeid(std::string) and rhs.type() == typeid(std::string))
    {
      auto l = std::any_cast<std::string>(lhs);
      auto r = std::any_cast<std::string>(rhs);
      std::string res;
      res.append(l);
      res.append(r);
      return res;
    }
    throw RuntimeException(expr->Operator, "operands must be number or string");
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
    checkNumberOperand(expr->Operator, lhs, rhs);
    return ! isEqual(lhs, rhs);
  case TokenType::EQUAL_EQUAL:
    checkNumberOperand(expr->Operator, lhs, rhs);
    return isEqual(lhs, rhs);
  default:
    return nullptr;
  }
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
  return env->get(expr->name);
}
