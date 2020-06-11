#ifndef Stmt_H_
#define Stmt_H_

#include "Token.h"
#include <any>
#include <vector>
using namespace lox;

class Stmt;
class Block;
class Class;
class Expression;
class Function;
class If;
class Print;
class Return;
class While;
class Var;

class StmtVisitor {
public:
  virtual ~StmtVisitor() {}
  virtual std::any     visitBlock      (Block      * Stmt) = 0;
  virtual std::any     visitClass      (Class      * Stmt) = 0;
  virtual std::any     visitExpression (Expression * Stmt) = 0;
  virtual std::any     visitFunction   (Function   * Stmt) = 0;
  virtual std::any     visitIf         (If         * Stmt) = 0;
  virtual std::any     visitPrint      (Print      * Stmt) = 0;
  virtual std::any     visitReturn     (Return     * Stmt) = 0;
  virtual std::any     visitWhile      (While      * Stmt) = 0;
  virtual std::any     visitVar        (Var        * Stmt) = 0;
};

class Stmt {
public:
  virtual ~Stmt() {}
  virtual std::any accept(StmtVisitor* visitor) = 0;
};

class Block       : public Stmt { 
public: 
  Block      (   std::vector<Stmt*> stmts)  :
    stmts(stmts) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitBlock      (this);
  }
public: 
   std::vector<Stmt*> stmts;
};

class Class       : public Stmt { 
public: 
  Class      (   Token name,    std::vector<Function*> methods)  :
    name(name), methods(methods) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitClass      (this);
  }
public: 
   Token name;
   std::vector<Function*> methods;
};

class Expression  : public Stmt { 
public: 
  Expression (   Expr* expr)  :
    expr(expr) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitExpression (this);
  }
public: 
   Expr* expr;
};

class Function    : public Stmt { 
public: 
  Function   (   Token name,    std::vector<Token> params,    std::vector<Stmt*> body)  :
    name(name), params(params), body(body) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitFunction   (this);
  }
public: 
   Token name;
   std::vector<Token> params;
   std::vector<Stmt*> body;
};

class If          : public Stmt { 
public: 
  If         (   Expr* condition,    Stmt* thenBranch,    Stmt* elseBranch)  :
    condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitIf         (this);
  }
public: 
   Expr* condition;
   Stmt* thenBranch;
   Stmt* elseBranch;
};

class Print       : public Stmt { 
public: 
  Print      (   Expr* expr)  :
    expr(expr) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitPrint      (this);
  }
public: 
   Expr* expr;
};

class Return      : public Stmt { 
public: 
  Return     (   Token keyword,    Expr* value)  :
    keyword(keyword), value(value) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitReturn     (this);
  }
public: 
   Token keyword;
   Expr* value;
};

class While       : public Stmt { 
public: 
  While      (   Expr* condition,    Stmt* body)  :
    condition(condition), body(body) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitWhile      (this);
  }
public: 
   Expr* condition;
   Stmt* body;
};

class Var         : public Stmt { 
public: 
  Var        (   Token name,    Expr* init)  :
    name(name), init(init) {}
  std::any accept(StmtVisitor* visitor) override {
    return visitor->visitVar        (this);
  }
public: 
   Token name;
   Expr* init;
};

#endif
