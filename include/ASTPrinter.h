#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace lox {
    class ASTPrinter : public ExprVisitor, StmtVisitor {
      public:
        std::any print(Expr* expr) {
            return expr->accept(this);
        }
        std::any print(Stmt* stmt) {
            return stmt->accept(this);
        }
        std::any print(std::vector<Expr*> exprs) {
            for (auto expr : exprs)
            {
                expr->accept(this);
                std::cout << ' ';
            }
            return nullptr;
        }
        std::any print(std::vector<Stmt*> stmts) {
            for (auto stmt : stmts)
            {
                stmt->accept(this);
                std::cout << '\n';
            }
            return nullptr;
        }
        std::any visitBlock(Block* stmt) override
        {
            std::cout << "(block\n";
            print(stmt->stmts);
            std::cout << ")";
            return nullptr;
            //return parenthesizeS("block", stmt->stmts);
        }
        std::any visitExpression(Expression* stmt) override
        {
            return parenthesizeE("", {stmt->expr});
        }
        std::any visitFunction(Function* stmt) override
        {
            std::string fn;
            fn.append("fn ");
            fn.append(stmt->name.lexeme);
            fn.append("(");
            for (auto p : stmt->params)
            {
                fn.append(p.lexeme);
                fn.append(" ");
            }
            fn.append(")");

            return parenthesizeS(fn, {stmt->body});
        }
        std::any visitIf(If* stmt) override
        {
            std::cout << "(if ";
            print(stmt->condition);
            return parenthesizeS("", {stmt->thenBranch, stmt->elseBranch});
        }
        std::any visitPrint(Print* stmt) override
        {
            return parenthesizeE("print", {stmt->expr});
        }
        std::any visitReturn(Return* Stmt) override
        {
            return parenthesizeE("return", {Stmt->value});
        }
        std::any visitWhile(While* Stmt) override
        {
            std::cout << "(while ";
            print(Stmt->condition);
            std::cout << '\n';
            print(Stmt->body);
            std::cout << ")";
            return nullptr;
        }
        std::any visitVar(Var* Stmt) override
        {
            return parenthesizeE("var " + Stmt->name.lexeme, {Stmt->init});
        }

        std::any visitAssign(Assign* expr) override
        {
            return parenthesizeE("= " + expr->name.lexeme, {expr->value});
        }
        std::any visitBinaryExpr(BinaryExpr* expr) override {
            return parenthesizeE(expr->Operator.lexeme,
                                {expr->left, expr->right});
        }
        std::any visitLogical(Logical* expr) override {
            return parenthesizeE(expr->Operator.lexeme,
                                {expr->left, expr->right});
        }
        std::any visitGroupingExpr(GroupingExpr* expr) override {
            return parenthesizeE("group", {expr->expression});
        }
        std::any visitCall(Call* Expr) override
        {
            // wrong
            std::cout << "(call ";
            print(Expr->callee);
            print(Expr->args);
            std::cout << ')';
            return nullptr;
        }
        std::any visitLiteralExpr(LiteralExpr* expr) override {
            std::cout << " " << expr->value;
            return nullptr;
        }
        std::any visitUnaryExpr(UnaryExpr* expr) override {
            return parenthesizeE(expr->Operator.lexeme, {expr->right});
        }
        std::any visitVariable(Variable* expr) override {
            std::cout << " " << expr->name.lexeme;
            return nullptr;
        }
        std::any parenthesizeS(std::string name, std::vector<Stmt*> v) {
            std::string pp = "(" + name;
            // print
            std::cout << pp;
            for (auto e : v) {
                if (e != nullptr)
                    e->accept(this);
            }
            std::cout << ")";
            return nullptr;
        }
        std::any parenthesizeE(std::string name, std::vector<Expr*> v) {
            std::string pp = "(" + name;
            // print
            std::cout << pp;
            for (auto e : v) {
                if (e != nullptr)
                    e->accept(this);
            }
            std::cout << ")";
            return nullptr;
        }
    };
}

/// EXAMPLE USE:
// int main() {
//     std::unique_ptr<Expr> rootExpr(
//         new BinaryExpr(new UnaryExpr(*new Token(TokenType::MINUS, "-", "",
//         1),
//                                      new LiteralExpr("123")),
//                        *new Token(TokenType::STAR, "*", "", 1),
//                        new GroupingExpr(new LiteralExpr("45.67"))));
//     ASTPrinter pp;
//     pp.print(rootExpr.get());
//     std::cout << std::endl;
//     return 0;
// }
