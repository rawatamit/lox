#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "Expr.h"
#include "Stmt.h"
#include "RuntimeException.h"
#include "Environment.h"
#include <memory>
#include <map>
#include <vector>

namespace lox
{

class Interpreter:
  public ExprVisitor,
  public StmtVisitor
{
private:
  std::shared_ptr<Environment> globals;
  std::shared_ptr<Environment> env;
  std::map<std::shared_ptr<Expr>, int> locals;

  void installNativeFns();
  void checkNumberOperand(Token tok, std::shared_ptr<LoxObject> expr);
  void checkNumberOperand(Token tok, std::shared_ptr<LoxObject> exp0, std::shared_ptr<LoxObject> exp1);
  std::shared_ptr<LoxObject> lookupVariable(const Token& name, std::shared_ptr<Expr> expr);

  virtual std::shared_ptr<LoxObject> execute(std::shared_ptr<Stmt> stmt);
  virtual std::shared_ptr<LoxObject> evaluate(std::shared_ptr<Expr> expr);
  virtual std::shared_ptr<LoxObject> visitClass(std::shared_ptr<Class> klass) override;
  virtual std::shared_ptr<LoxObject> visitFunction(std::shared_ptr<Function> stmt) override;
  virtual std::shared_ptr<LoxObject> visitExpression(std::shared_ptr<Expression> stmt) override;
  virtual std::shared_ptr<LoxObject> visitIf(std::shared_ptr<If> stmt) override;
  virtual std::shared_ptr<LoxObject> visitPrint(std::shared_ptr<Print> stmt) override;
  virtual std::shared_ptr<LoxObject> visitWhile(std::shared_ptr<While> stmt) override;
  virtual std::shared_ptr<LoxObject> visitReturn(std::shared_ptr<Return> stmt) override;
  virtual std::shared_ptr<LoxObject> visitVar(std::shared_ptr<Var> stmt) override;
  virtual std::shared_ptr<LoxObject> visitBlock(std::shared_ptr<Block> stmt) override;
  virtual std::shared_ptr<LoxObject> visitLogical(std::shared_ptr<Logical> expr) override;
  virtual std::shared_ptr<LoxObject> visitAssign(std::shared_ptr<Assign> expr) override;
  virtual std::shared_ptr<LoxObject> visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitCall(std::shared_ptr<Call> expr) override;
  virtual std::shared_ptr<LoxObject> visitGet(std::shared_ptr<Get> expr) override;
  virtual std::shared_ptr<LoxObject> visitSet(std::shared_ptr<Set> expr) override;
  virtual std::shared_ptr<LoxObject> visitThis(std::shared_ptr<This> expr) override;
  virtual std::shared_ptr<LoxObject> visitSuper(std::shared_ptr<Super> expr) override;
  virtual std::shared_ptr<LoxObject> visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitVariable(std::shared_ptr<Variable> expr) override;

public:
  Interpreter() :
    globals(std::make_shared<Environment>()),
    env(globals)
  {
    installNativeFns();
  }

  void resolve(std::shared_ptr<Expr> expr, int depth);
  virtual void interpret(std::vector<std::shared_ptr<Stmt>> stmts);
  std::shared_ptr<LoxObject> executeBlock(
    std::vector<std::shared_ptr<Stmt>>& stmt,
    std::shared_ptr<Environment> nenv);
  std::shared_ptr<Environment> getGlobalEnvironment() const
  { return globals; }
};

} // namespace 

#endif
