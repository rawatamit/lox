#ifndef Stmt_H_
#define Stmt_H_

#include "Token.h"
#include <any>
#include <vector>
using namespace lox;

class Stmt;
class Block;
class Expression;
class Print;
class Var;

class StmtVisitor {
public:
  virtual ~StmtVisitor() {}
  virtual std::any     visitBlock      (Block      * Stmt) = 0;
  virtual std::any     visitExpression   (Expression   * Stmt) = 0;
  virtual std::any     visitPrint (Print * Stmt) = 0;
  virtual std::any     visitVar        (Var        * Stmt) = 0;
};

class Stmt {
public:
  virtual ~Stmt() {}
  virtual std::any accept(StmtVisitor* visitor) = 0;
};

class Block       : public Stmt { 
public: 
  Block      (   std::vector<Stmt*> stmts
)  : stmts(stmts) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitBlock      (this);
  }
public: 
   std::vector<Stmt*> stmts;
};

class Expression    : public Stmt { 
public: 
  Expression   (  Expr* expr
)  : expr(expr) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitExpression   (this);
  }
public: 
  Expr* expr;
};

class Print  : public Stmt { 
public: 
  Print (  Expr* expr
)  : expr(expr) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitPrint (this);
  }
public: 
  Expr* expr;
};

class Var         : public Stmt { 
public: 
  Var        (   Token name
,    Expr* init
)  : name(name), init(init) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitVar        (this);
  }
public: 
   Token name;
   Expr* init;
};

#endif
