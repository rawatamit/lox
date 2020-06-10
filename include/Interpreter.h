#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "Expr.h"
#include "Stmt.h"
#include "RuntimeException.h"
#include "Environment.h"
#include <any>
#include <memory>
#include <map>
#include <vector>

namespace lox
{

class Interpreter: public ExprVisitor, public StmtVisitor
{
private:
  std::shared_ptr<Environment> globals;
  std::shared_ptr<Environment> env;
  std::map<Expr*, int> locals;

  void installNativeFns();
  void checkNumberOperand(Token tok, std::any expr);
  void checkNumberOperand(Token tok, std::any exp0, std::any exp1);
  std::any lookupVariable(const Token& name, Expr* expr);

  virtual std::any execute(Stmt* stmt);
  virtual std::any evaluate(Expr* expr);
  virtual std::any visitFunction(Function* stmt) override;
  virtual std::any visitExpression(Expression* stmt) override;
  virtual std::any visitIf(If* stmt) override;
  virtual std::any visitPrint(Print* stmt) override;
  virtual std::any visitWhile(While* stmt) override;
  virtual std::any visitReturn(Return* stmt) override;
  virtual std::any visitVar(Var* stmt) override;
  virtual std::any visitBlock(Block* stmt) override;
  virtual std::any visitLogical(Logical* expr) override;
  virtual std::any visitAssign(Assign* expr) override;
  virtual std::any visitBinaryExpr(BinaryExpr* expr) override;
  virtual std::any visitCall(Call* expr) override;
  virtual std::any visitGroupingExpr(GroupingExpr* expr) override;
  virtual std::any visitLiteralExpr(LiteralExpr* expr) override;
  virtual std::any visitUnaryExpr(UnaryExpr* expr) override;
  virtual std::any visitVariable(Variable* expr) override;

public:
  Interpreter() :
    globals(std::make_shared<Environment>()),
    env(globals)
  {
    installNativeFns();
  }

  void resolve(Expr* expr, int depth);
  virtual void interpret(std::vector<Stmt*> stmts);
  std::any executeBlock(
    std::vector<Stmt*>& stmt,
    std::shared_ptr<Environment> nenv);
  std::shared_ptr<Environment> getGlobalEnvironment() const
  { return globals; }
};

} // namespace 

#endif
