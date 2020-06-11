#ifndef Expr_H_
#define Expr_H_

#include "Token.h"
#include "lox/LoxObject.h"
#include <memory>
#include <vector>
using namespace lox;

class Expr;
class Assign;
class BinaryExpr;
class Call;
class Get;
class GroupingExpr;
class LiteralExpr;
class Logical;
class Set;
class This;
class Super;
class UnaryExpr;
class Variable;

class ExprVisitor {
public:
  virtual ~ExprVisitor() {}
  virtual std::shared_ptr<LoxObject>     visitAssign       (std::shared_ptr<Assign       > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitBinaryExpr   (std::shared_ptr<BinaryExpr   > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitCall     (std::shared_ptr<Call     > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitGet      (std::shared_ptr<Get      > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitGroupingExpr (std::shared_ptr<GroupingExpr > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitLiteralExpr  (std::shared_ptr<LiteralExpr  > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitLogical  (std::shared_ptr<Logical  > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitSet      (std::shared_ptr<Set      > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitThis     (std::shared_ptr<This     > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitSuper    (std::shared_ptr<Super    > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitUnaryExpr    (std::shared_ptr<UnaryExpr    > Expr) = 0;
  virtual std::shared_ptr<LoxObject>     visitVariable     (std::shared_ptr<Variable     > Expr) = 0;
};

class Expr {
public:
  virtual ~Expr() {}
  virtual std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) = 0;
};

class Assign        : public std::enable_shared_from_this<Assign       >, public Expr { 
public: 
  Assign       (  Token name,    std::shared_ptr<Expr> value)  :
    name(name), value(value) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Assign       > p{shared_from_this()};
    return visitor.visitAssign       (p);
  }
public: 
  Token name;
   std::shared_ptr<Expr> value;
};

class BinaryExpr    : public std::enable_shared_from_this<BinaryExpr   >, public Expr { 
public: 
  BinaryExpr   (  std::shared_ptr<Expr> left,    Token Operator,    std::shared_ptr<Expr> right)  :
    left(left), Operator(Operator), right(right) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<BinaryExpr   > p{shared_from_this()};
    return visitor.visitBinaryExpr   (p);
  }
public: 
  std::shared_ptr<Expr> left;
   Token Operator;
   std::shared_ptr<Expr> right;
};

class Call      : public std::enable_shared_from_this<Call     >, public Expr { 
public: 
  Call     (   std::shared_ptr<Expr> callee,    Token paren,    std::vector<std::shared_ptr<Expr>> args)  :
    callee(callee), paren(paren), args(args) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Call     > p{shared_from_this()};
    return visitor.visitCall     (p);
  }
public: 
   std::shared_ptr<Expr> callee;
   Token paren;
   std::vector<std::shared_ptr<Expr>> args;
};

class Get       : public std::enable_shared_from_this<Get      >, public Expr { 
public: 
  Get      (   std::shared_ptr<Expr> object,    Token name)  :
    object(object), name(name) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Get      > p{shared_from_this()};
    return visitor.visitGet      (p);
  }
public: 
   std::shared_ptr<Expr> object;
   Token name;
};

class GroupingExpr  : public std::enable_shared_from_this<GroupingExpr >, public Expr { 
public: 
  GroupingExpr (   std::shared_ptr<Expr> expression)  :
    expression(expression) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<GroupingExpr > p{shared_from_this()};
    return visitor.visitGroupingExpr (p);
  }
public: 
   std::shared_ptr<Expr> expression;
};

class LiteralExpr   : public std::enable_shared_from_this<LiteralExpr  >, public Expr { 
public: 
  LiteralExpr  (  TokenType type,    std::string value)  :
    type(type), value(value) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<LiteralExpr  > p{shared_from_this()};
    return visitor.visitLiteralExpr  (p);
  }
public: 
  TokenType type;
   std::string value;
};

class Logical   : public std::enable_shared_from_this<Logical  >, public Expr { 
public: 
  Logical  (   std::shared_ptr<Expr> left,    Token Operator,    std::shared_ptr<Expr> right)  :
    left(left), Operator(Operator), right(right) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Logical  > p{shared_from_this()};
    return visitor.visitLogical  (p);
  }
public: 
   std::shared_ptr<Expr> left;
   Token Operator;
   std::shared_ptr<Expr> right;
};

class Set       : public std::enable_shared_from_this<Set      >, public Expr { 
public: 
  Set      (   std::shared_ptr<Expr> object,    Token name,    std::shared_ptr<Expr> value)  :
    object(object), name(name), value(value) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Set      > p{shared_from_this()};
    return visitor.visitSet      (p);
  }
public: 
   std::shared_ptr<Expr> object;
   Token name;
   std::shared_ptr<Expr> value;
};

class This      : public std::enable_shared_from_this<This     >, public Expr { 
public: 
  This     (   Token keyword)  :
    keyword(keyword) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<This     > p{shared_from_this()};
    return visitor.visitThis     (p);
  }
public: 
   Token keyword;
};

class Super     : public std::enable_shared_from_this<Super    >, public Expr { 
public: 
  Super    (   Token keyword,    Token method)  :
    keyword(keyword), method(method) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Super    > p{shared_from_this()};
    return visitor.visitSuper    (p);
  }
public: 
   Token keyword;
   Token method;
};

class UnaryExpr     : public std::enable_shared_from_this<UnaryExpr    >, public Expr { 
public: 
  UnaryExpr    (  Token Operator,    std::shared_ptr<Expr> right)  :
    Operator(Operator), right(right) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<UnaryExpr    > p{shared_from_this()};
    return visitor.visitUnaryExpr    (p);
  }
public: 
  Token Operator;
   std::shared_ptr<Expr> right;
};

class Variable      : public std::enable_shared_from_this<Variable     >, public Expr { 
public: 
  Variable     (  Token name)  :
    name(name) {}
  std::shared_ptr<LoxObject> accept(ExprVisitor& visitor) override {
    std::shared_ptr<Variable     > p{shared_from_this()};
    return visitor.visitVariable     (p);
  }
public: 
  Token name;
};

#endif
