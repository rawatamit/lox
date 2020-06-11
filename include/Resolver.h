#ifndef _RESOLVER_H_
#define _RESOLVER_H_

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Interpreter.h"
#include "ErrorHandler.h"

#include <any>
#include <map>
#include <vector>

namespace lox
{

class Resolver : public ExprVisitor, public StmtVisitor
{
private:
  enum FunctionType
  {
    NONEF,
    FUNCTION,
    METHOD,
    INITIALIZER,
  };

  enum ClassType
  {
    NONEC,
    CLASS,
  };

private:
  std::shared_ptr<Interpreter> interpreter;
  ErrorHandler& errorHandler;
  FunctionType currentFunction;
  ClassType currentClass;
  std::vector<std::map<std::string, bool>> scopes;

public:
  Resolver(std::shared_ptr<Interpreter> interpreter,
           ErrorHandler& errorHandler) :
    interpreter(interpreter),
    errorHandler(errorHandler),
    currentFunction(NONEF),
    currentClass(NONEC)
  {}

  ~Resolver() = default;
  void resolve(std::shared_ptr<Stmt> stmt);
  void resolve(std::shared_ptr<Expr> expr);
  void resolve(const std::vector<std::shared_ptr<Stmt>>& stmts);
  void resolveLocal(std::shared_ptr<Expr> expr, const Token& tok);
  void resolveFunction(std::shared_ptr<Function> fn, FunctionType type);
  void beginScope();
  void endScope();
  void declare(const Token& tok);
  void define(const Token& name);

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
  virtual std::shared_ptr<LoxObject> visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) override;
  virtual std::shared_ptr<LoxObject> visitVariable(std::shared_ptr<Variable> expr) override;
};

} // namespace

#endif
