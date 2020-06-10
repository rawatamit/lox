#ifndef Expr_H_
#define Expr_H_

#include "Token.h"
#include <any>
#include <vector>
using namespace lox;

class Expr;
class Assign;
class BinaryExpr;
class Call;
class GroupingExpr;
class LiteralExpr;
class Logical;
class UnaryExpr;
class Variable;

class ExprVisitor {
public:
  virtual ~ExprVisitor() {}
  virtual std::any     visitAssign       (Assign       * Expr) = 0;
  virtual std::any     visitBinaryExpr   (BinaryExpr   * Expr) = 0;
  virtual std::any     visitCall     (Call     * Expr) = 0;
  virtual std::any     visitGroupingExpr (GroupingExpr * Expr) = 0;
  virtual std::any     visitLiteralExpr  (LiteralExpr  * Expr) = 0;
  virtual std::any     visitLogical  (Logical  * Expr) = 0;
  virtual std::any     visitUnaryExpr    (UnaryExpr    * Expr) = 0;
  virtual std::any     visitVariable     (Variable     * Expr) = 0;
};

class Expr {
public:
  virtual ~Expr() {}
  virtual std::any accept(ExprVisitor* visitor) = 0;
};

class Assign        : public Expr { 
public: 
  Assign       (  Token name,    Expr* value)  :
    name(name), value(value) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitAssign       (this);
  }
public: 
  Token name;
   Expr* value;
};

class BinaryExpr    : public Expr { 
public: 
  BinaryExpr   (  Expr* left,    Token Operator,    Expr* right)  :
    left(left), Operator(Operator), right(right) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitBinaryExpr   (this);
  }
public: 
  Expr* left;
   Token Operator;
   Expr* right;
};

class Call      : public Expr { 
public: 
  Call     (   Expr* callee,    Token paren,    std::vector<Expr*> args)  :
    callee(callee), paren(paren), args(args) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitCall     (this);
  }
public: 
   Expr* callee;
   Token paren;
   std::vector<Expr*> args;
};

class GroupingExpr  : public Expr { 
public: 
  GroupingExpr (  Expr* expression)  :
    expression(expression) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitGroupingExpr (this);
  }
public: 
  Expr* expression;
};

class LiteralExpr   : public Expr { 
public: 
  LiteralExpr  (  TokenType type,    std::string value)  :
    type(type), value(value) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitLiteralExpr  (this);
  }
public: 
  TokenType type;
   std::string value;
};

class Logical   : public Expr { 
public: 
  Logical  (   Expr* left,    Token Operator,    Expr* right)  :
    left(left), Operator(Operator), right(right) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitLogical  (this);
  }
public: 
   Expr* left;
   Token Operator;
   Expr* right;
};

class UnaryExpr     : public Expr { 
public: 
  UnaryExpr    (  Token Operator,    Expr* right)  :
    Operator(Operator), right(right) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitUnaryExpr    (this);
  }
public: 
  Token Operator;
   Expr* right;
};

class Variable      : public Expr { 
public: 
  Variable     (  Token name)  :
    name(name) {}
  std::any accept(ExprVisitor* visitor) override {
    return visitor->visitVariable     (this);
  }
public: 
  Token name;
};

#endif
