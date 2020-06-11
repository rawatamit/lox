#include "Resolver.h"

void Resolver::resolve(Stmt* stmt)
{
  stmt->accept(this);
}

void Resolver::resolve(Expr* expr)
{
  expr->accept(this);
}

void Resolver::resolve(const std::vector<Stmt*>& stmts)
{
  for (Stmt* stmt : stmts)
  {
    resolve(stmt);
  }
}

void Resolver::resolveLocal(Expr* expr, const Token& tok)
{
  for (int i = scopes.size() - 1; i >= 0; --i)
  {
    if (scopes[i].find(tok.lexeme) != scopes[i].end())
    {
      interpreter->resolve(expr, scopes.size() - 1 - i);
      return;
    }
  }

  // not found, assume global
}

void Resolver::resolveFunction(Function* fn, FunctionType type)
{
  FunctionType enclosingFn = currentFunction;
  currentFunction = type;
  beginScope();
  for (auto& param : fn->params)
  {
    declare(param);
    define(param);
  }
  resolve(fn->body);
  endScope();
  currentFunction = enclosingFn;
}

void Resolver::beginScope()
{
  scopes.push_back(std::map<std::string, bool>());
}

void Resolver::endScope()
{
  scopes.pop_back();
}

void Resolver::declare(const Token& name)
{
  if (!scopes.empty())
  {
    if (scopes.back().find(name.lexeme) != scopes.back().end())
    {
      errorHandler.add(name.line, " at '" + name.lexeme + "'",
        "Variable with this name already declared in this scope.");
    }
    scopes.back()[name.lexeme] = false;
  }
}

void Resolver::define(const Token& name)
{
  if (!scopes.empty())
  {
    scopes.back()[name.lexeme] = true;
  }
}

std::any Resolver::visitClass(Class* klass)
{
  ClassType enclosingClass = currentClass;
  currentClass = ClassType::CLASS;
  declare(klass->name);
  define(klass->name);

  // define this
  beginScope();
  scopes.back()["this"] = true;

  for (Function* method : klass->methods)
  {
    FunctionType decl =
      (method->name.lexeme == "this")
      ? INITIALZER : METHOD;
    resolveFunction(method, decl);
  }

  endScope();
  currentClass = enclosingClass;
  return nullptr;
}

std::any Resolver::visitFunction(Function* stmt)
{
  declare(stmt->name);
  define(stmt->name);
  resolveFunction(stmt, FUNCTION);
  return nullptr;
}

std::any Resolver::visitExpression(Expression* stmt)
{
  resolve(stmt->expr);
  return nullptr;
}

std::any Resolver::visitIf(If* stmt)
{
  resolve(stmt->condition);
  resolve(stmt->thenBranch);
  if (stmt->elseBranch != nullptr)
  {
    resolve(stmt->elseBranch);
  }
  return nullptr;
}

std::any Resolver::visitPrint(Print* stmt)
{
  resolve(stmt->expr);
  return nullptr;
}

std::any Resolver::visitWhile(While* stmt)
{
  resolve(stmt->condition);
  resolve(stmt->body);
  return nullptr;
}

std::any Resolver::visitReturn(Return* stmt)
{
  if (currentFunction != FUNCTION && currentFunction != METHOD)
  {
    errorHandler.add(stmt->keyword.line, " at 'return'",
      "Cannot return from top-level code.");
  }

  if (stmt->value != nullptr)
  {
    if (currentFunction == INITIALZER)
    {
      errorHandler.add(stmt->keyword.line, " at 'return'",
        "Cannot return a value from an initializer.");
    }
    resolve(stmt->value);
  }

  return nullptr;
}

std::any Resolver::visitVar(Var* stmt)
{
  declare(stmt->name);
  if (stmt->init != nullptr)
  {
    resolve(stmt->init);
  }
  define(stmt->name);
  return nullptr;
}

std::any Resolver::visitBlock(Block* stmt)
{
  beginScope();
  resolve(stmt->stmts);
  endScope();
  return nullptr;
}

std::any Resolver::visitLogical(Logical* expr)
{
  resolve(expr->left);
  resolve(expr->right);
  return nullptr;
}

std::any Resolver::visitAssign(Assign* expr)
{
  resolve(expr->value);
  resolveLocal(expr, expr->name);
  return nullptr;
}

std::any Resolver::visitBinaryExpr(BinaryExpr* expr)
{
  resolve(expr->left);
  resolve(expr->right);
  return nullptr;
}

std::any Resolver::visitCall(Call* expr)
{
  resolve(expr->callee);
  for (Expr* arg : expr->args)
  {
    resolve(arg);
  }
  return nullptr;
}

std::any Resolver::visitGet(Get* expr)
{
  resolve(expr->object);
  return nullptr;
}

std::any Resolver::visitSet(Set* expr)
{
  resolve(expr->value);
  resolve(expr->object);
  return nullptr;
}

std::any Resolver::visitThis(This* expr)
{
  if (currentClass == NONEC)
  {
    errorHandler.add(expr->keyword.line,
      " at '" + expr->keyword.lexeme + "'",
      "Cannot use 'this' outside of a class.");
    return nullptr;
  }

  resolveLocal(expr, expr->keyword);
  return nullptr;
}

std::any Resolver::visitGroupingExpr(GroupingExpr* expr)
{
  resolve(expr->expression);
  return nullptr;
}

std::any Resolver::visitLiteralExpr(LiteralExpr*)
{
  return nullptr;
}

std::any Resolver::visitUnaryExpr(UnaryExpr* expr)
{
  resolve(expr->right);
  return nullptr;
}

std::any Resolver::visitVariable(Variable* expr)
{
  if (!scopes.empty())
  {
    auto it = scopes.back().find(expr->name.lexeme);
    if (it != scopes.back().end() and it->second == false)
    {
      errorHandler.add(
        expr->name.line,
        " at '" + expr->name.lexeme + "'",
        "Cannot read local variable in its own initialiser.");
    }
  }

  resolveLocal(expr, expr->name);
  return nullptr;
}
