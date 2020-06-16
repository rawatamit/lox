#include "Resolver.h"

void Resolver::resolve(std::shared_ptr<Stmt> stmt) { stmt->accept(*this); }

void Resolver::resolve(std::shared_ptr<Expr> expr) { expr->accept(*this); }

void Resolver::resolve(const std::vector<std::shared_ptr<Stmt>> &stmts) {
  for (auto stmt : stmts) {
    resolve(stmt);
  }
}

void Resolver::resolveLocal(std::shared_ptr<Expr> expr, const Token &tok) {
  for (int i = scopes.size() - 1; i >= 0; --i) {
    if (scopes[i].find(tok.lexeme) != scopes[i].end()) {
      interpreter->resolve(expr, scopes.size() - 1 - i);
      return;
    }
  }

  // not found, assume global
}

void Resolver::resolveFunction(std::shared_ptr<Function> fn,
                               FunctionType type) {
  FunctionType enclosingFn = currentFunction;
  currentFunction = type;
  beginScope();
  for (auto &param : fn->params) {
    declare(param);
    define(param);
  }
  resolve(fn->body);
  endScope();
  currentFunction = enclosingFn;
}

void Resolver::beginScope() { scopes.push_back(std::map<std::string, bool>()); }

void Resolver::endScope() { scopes.pop_back(); }

void Resolver::declare(const Token &name) {
  if (!scopes.empty()) {
    if (scopes.back().find(name.lexeme) != scopes.back().end()) {
      errorHandler.add(
          name.line, " at '" + name.lexeme + "'",
          "Variable with this name already declared in this scope.");
    }
    scopes.back()[name.lexeme] = false;
  }
}

void Resolver::define(const Token &name) {
  if (!scopes.empty()) {
    scopes.back()[name.lexeme] = true;
  }
}

std::shared_ptr<lox::LoxObject>
Resolver::visitClass(std::shared_ptr<Class> klass) {
  ClassType enclosingClass = currentClass;
  currentClass = ClassType::CLASS;
  declare(klass->name);
  define(klass->name);

  if (klass->superclass != nullptr) {
    if (klass->superclass->name.lexeme == klass->name.lexeme) {
      errorHandler.add(klass->name.line, " at 'return'",
                       "A class cannot inherit from itself.");
    } else {
      currentClass = SUBCLASS;
      resolve(klass->superclass);
      beginScope();
      scopes.back()["super"] = true;
    }
  }

  // define this
  beginScope();
  scopes.back()["this"] = true;

  for (auto method : klass->methods) {
    FunctionType decl = (method->name.lexeme == "init") ? INITIALIZER : METHOD;
    resolveFunction(method, decl);
  }

  endScope();
  if (klass->superclass != nullptr) {
    endScope();
  }

  currentClass = enclosingClass;
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitFunction(std::shared_ptr<Function> stmt) {
  declare(stmt->name);
  define(stmt->name);
  resolveFunction(stmt, FUNCTION);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitExpression(std::shared_ptr<Expression> stmt) {
  resolve(stmt->expr);
  return nullptr;
}

std::shared_ptr<lox::LoxObject> Resolver::visitIf(std::shared_ptr<If> stmt) {
  resolve(stmt->condition);
  resolve(stmt->thenBranch);
  if (stmt->elseBranch != nullptr) {
    resolve(stmt->elseBranch);
  }
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitPrint(std::shared_ptr<Print> stmt) {
  resolve(stmt->expr);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitWhile(std::shared_ptr<While> stmt) {
  resolve(stmt->condition);
  resolve(stmt->body);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitReturn(std::shared_ptr<Return> stmt) {
  if (currentFunction == NONEF) {
    errorHandler.add(stmt->keyword.line, " at 'return'",
                     "Cannot return from top-level code.");
  }

  if (stmt->value != nullptr) {
    if (currentFunction == INITIALIZER) {
      errorHandler.add(stmt->keyword.line, " at 'return'",
                       "Cannot return a value from an initialiser.");
    } else {
      resolve(stmt->value);
    }
  }

  return nullptr;
}

std::shared_ptr<lox::LoxObject> Resolver::visitVar(std::shared_ptr<Var> stmt) {
  declare(stmt->name);
  if (stmt->init != nullptr) {
    resolve(stmt->init);
  }
  define(stmt->name);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitBlock(std::shared_ptr<Block> stmt) {
  beginScope();
  resolve(stmt->stmts);
  endScope();
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitLogical(std::shared_ptr<Logical> expr) {
  resolve(expr->left);
  resolve(expr->right);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitAssign(std::shared_ptr<Assign> expr) {
  resolve(expr->value);
  resolveLocal(expr, expr->name);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) {
  resolve(expr->left);
  resolve(expr->right);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitCall(std::shared_ptr<Call> expr) {
  resolve(expr->callee);
  for (auto arg : expr->args) {
    resolve(arg);
  }
  return nullptr;
}

std::shared_ptr<lox::LoxObject> Resolver::visitGet(std::shared_ptr<Get> expr) {
  resolve(expr->object);
  return nullptr;
}

std::shared_ptr<lox::LoxObject> Resolver::visitSet(std::shared_ptr<Set> expr) {
  resolve(expr->value);
  resolve(expr->object);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitThis(std::shared_ptr<This> expr) {
  if (currentClass == NONEC) {
    errorHandler.add(expr->keyword.line, " at '" + expr->keyword.lexeme + "'",
                     "Cannot use 'this' outside of a class.");
    return nullptr;
  }

  resolveLocal(expr, expr->keyword);
  return nullptr;
}

std::shared_ptr<LoxObject> Resolver::visitSuper(std::shared_ptr<Super> expr) {
  if (currentClass == NONEC) {
    errorHandler.add(expr->keyword.line, " at '" + expr->keyword.lexeme + "'",
                     "Cannot use 'super' outside of a class.");
  } else if (currentClass != SUBCLASS) {
    errorHandler.add(expr->keyword.line, " at '" + expr->keyword.lexeme + "'",
                     "Cannot use 'super' in a class without a superclass.");
  }

  resolveLocal(expr, expr->keyword);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) {
  resolve(expr->expression);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitLiteralExpr(std::shared_ptr<LiteralExpr>) {
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) {
  resolve(expr->right);
  return nullptr;
}

std::shared_ptr<lox::LoxObject>
Resolver::visitVariable(std::shared_ptr<Variable> expr) {
  if (!scopes.empty()) {
    auto it = scopes.back().find(expr->name.lexeme);
    if (it != scopes.back().end() and it->second == false) {
      errorHandler.add(expr->name.line, " at '" + expr->name.lexeme + "'",
                       "Cannot read local variable in its own initialiser.");
    }
  }

  resolveLocal(expr, expr->name);
  return nullptr;
}
