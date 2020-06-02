#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "Expr.h"
#include "Stmt.h"
#include "RuntimeException.h"
#include "Environment.h"
#include <any>
#include <memory>
#include <vector>

namespace lox
{

class Interpreter: public ExprVisitor, public StmtVisitor
{
private:
  std::shared_ptr<Environment> env;

  void checkNumberOperand(Token tok, std::any expr)
  {
    if (expr.type() != typeid(double))
    {
      throw RuntimeException(tok, "need a number operand");
    }
  }

  void checkNumberOperand(Token tok, std::any exp0, std::any exp1)
  {
    if (exp0.type() != typeid(double) or
        exp1.type() != typeid(double))
    {
      throw RuntimeException(tok, "need number operands");
    }
  }

  bool isTruthy(std::any expr)
  {
    if (expr.type() == typeid(std::nullptr_t))
    {
      return false;
    }

    if (expr.type() == typeid(bool))
    {
      return std::any_cast<bool>(expr);
    }

    return true;
  }

  bool isEqual(std::any arg0, std::any arg1)
  {
    if (arg0.type() == typeid(std::nullptr_t) and
        arg1.type() == typeid(std::nullptr_t))
    {
      return true;
    }
    else if (arg0.type() == typeid(std::nullptr_t) or
             arg1.type() == typeid(std::nullptr_t))
    {
      return false;
    }
    else if (arg0.type() == typeid(double) and
             arg1.type() == typeid(double))
    {
      auto e0 = std::any_cast<double>(arg0);
      auto e1 = std::any_cast<double>(arg1);

      return e0 == e1;
    }
    else if (arg0.type() == typeid(std::string) and
             arg1.type() == typeid(std::string))
    {
      auto e0 = std::any_cast<std::string>(arg0);
      auto e1 = std::any_cast<std::string>(arg1);

      return e0 == e1;
    }
    else if (arg0.type() == typeid(bool) and
             arg1.type() == typeid(bool))
    {
      auto e0 = std::any_cast<bool>(arg0);
      auto e1 = std::any_cast<bool>(arg1);

      return e0 == e1;
    }

    return false;
  }

  std::string stringify(std::any r)
  {
    if (r.type() == typeid(std::nullptr_t))
    {
      return "nil";
    }
    else if (r.type() == typeid(bool))
    {
      return std::any_cast<bool>(r) ? "true" : "false";
    }
    else if (r.type() == typeid(double))
    {
      return std::to_string(std::any_cast<double>(r));
    }
    else if (r.type() == typeid(std::string))
    {
      std::string ret;
      ret.push_back('"');
      ret.append(std::any_cast<std::string>(r));
      ret.push_back('"');
      return ret;
    }
    else
    {
      throw RuntimeException("stringify()");
    }
  }

  virtual std::any execute(Stmt* stmt);
  virtual std::any evaluate(Expr* expr);
  virtual std::any visitExpression(Expression* stmt) override;
  virtual std::any visitPrint(Print* stmt) override;
  virtual std::any visitVar(Var* stmt) override;
  virtual std::any visitBlock(Block* stmt) override;
  virtual std::any visitAssign(Assign* expr) override;
  virtual std::any visitBinaryExpr(BinaryExpr* expr) override;
  virtual std::any visitGroupingExpr(GroupingExpr* expr) override;
  virtual std::any visitLiteralExpr(LiteralExpr* expr) override;
  virtual std::any visitUnaryExpr(UnaryExpr* expr) override;
  virtual std::any visitVariable(Variable* expr) override;

public:
  Interpreter() :
    env(std::make_shared<Environment>())
  {}
  virtual void interpret(std::vector<Stmt*> stmts);
};

} // namespace 

#endif
