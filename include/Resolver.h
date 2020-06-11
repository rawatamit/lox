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
    INITIALZER,
  };

  enum ClassType
  {
    NONEC,
    CLASS,
  };

private:
  Interpreter* interpreter;
  ErrorHandler& errorHandler;
  FunctionType currentFunction;
  ClassType currentClass;
  std::vector<std::map<std::string, bool>> scopes;

public:
  Resolver(Interpreter* interpreter,
           ErrorHandler& errorHandler) :
    interpreter(interpreter),
    errorHandler(errorHandler),
    currentFunction(NONEF),
    currentClass(NONEC)
  {}

  ~Resolver() = default;
  void resolve(Stmt* stmt);
  void resolve(Expr* expr);
  void resolve(const std::vector<Stmt*>& stmts);
  void resolveLocal(Expr* expr, const Token& tok);
  void resolveFunction(Function* fn, FunctionType type);
  void beginScope();
  void endScope();
  void declare(const Token& tok);
  void define(const Token& name);

  virtual std::any visitClass(Class* klass) override;
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
  virtual std::any visitGet(Get* expr) override;
  virtual std::any visitSet(Set* expr) override;
  virtual std::any visitThis(This* expr) override;
  virtual std::any visitGroupingExpr(GroupingExpr* expr) override;
  virtual std::any visitLiteralExpr(LiteralExpr* expr) override;
  virtual std::any visitUnaryExpr(UnaryExpr* expr) override;
  virtual std::any visitVariable(Variable* expr) override;
};

} // namespace

#endif
