#pragma once

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

#include "ast/type.hpp"
#include "ast/astvisitor.hpp"

#define ACCEPT_FUNCTION void accept(ASTVisitor& v) override {v.visit(*this);}

namespace rulejit {

struct AST{
    virtual void accept(ASTVisitor&) = 0;
    virtual ~AST() = default;
};

// EXPR := LEXPR | LITERAL | FUNCCALL | COMPLEX | BRANCH | LOOP | BLOCK
struct ExprAST : public AST {
    std::unique_ptr<TypeInfo> type;
};

// LEXPR := IDENT | MEMBER
struct AssignableExprAST : public ExprAST {};

// IDENT
// x
struct IdentifierExprAST : public AssignableExprAST{
    ACCEPT_FUNCTION;
    std::string name;
};

// MEMBER := EXPR '.' IDENT | EXPR '[' EXPR ']'
// vector.x (same as vector["x"]) | list[a+b] | make_vector(1, 2, 3).x
struct MemberAccessExprAST : public AssignableExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> baseVar;
    std::unique_ptr<ExprAST> memberToken;
};

// LITERAL
// "abc" | 12 | 1e3 TODO: | constexpr
struct LiteralExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string value;
};

// FUNCCALL := IDENT '(' (EXPR (',' EXPR)*)? ')'
// add(1+3)
struct FunctionCallExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string functionName;
    std::vector<std::unique_ptr<ExprAST>> params;
};

// COMPLEX := (SLICETYPE | ARRAYTYPE) '{' (EXPR (',' EXPR)*)? '}' | IDENT '{' (IDENT ':' EXPR (',' IDENT ':' EXPR)*)? '}'
// []i64{1, 2, 3} | Info{base: Base{name: "abc", value: 3}, time: 13} | Vector3{x: 1, y: 2, z: 3}
struct ComplexLiteralExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<ExprAST>> members;
};

// BRANCH := 'if' '(' EXPR ')' EXPR 'else' EXPR
// if(a==0){1}else{2} | if(a) 1 else 4
struct BranchExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> trueBranch;
    std::unique_ptr<ExprAST> falseBranch;
};

// LOOP := 'while' '(' EXPR ')' EXPR
// while(x!=0){x+=1;x;}
struct LoopAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> body;
};

// BLOCK := '{' ((EXPR | DEF | ASSIGN) ENDLINE)* EXPR '}'
// {var x i64 = 12; x;}
struct BlockExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> preStatement;
    std::unique_ptr<ExprAST> value;
};

// ASSIGN := LEXPR '=' EXPR ENDLINE
// x = 12 CAUTION: no returns and not an expression
struct AssignmentAST : public AST{
    ACCEPT_FUNCTION;
    std::unique_ptr<AssignableExprAST> target;
    std::unique_ptr<ExprAST> value;
};

// DEF := TYPEDEF | VARDEF | FUNCDEF
struct DefAST : public AST {
    std::string name;
};

// TYPEDEF := 'type' IDENT TYPE ('|' TYPE)* ENDLINE
// type Vector3 struct {x f64; y f64; z f64;} | type double f64 | TODO: type Reference<T> class {T item;}
struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    struct VarDef {
        std::string type;
        std::string name;
    };
    std::vector<VarDef> member;
};

// VARDEF := 'var' IDENT TYPE ('='? EXPR)? ENDLINE | 'var' IDENT ':=' EXPR ENDLINE
// var x []i64 = {1, 3, 4};
// TODO: var x [4]f64;
// TODO: var x [-]i64 {1, 3, 4};
// TODO: var x []i64 {1, 3, 4};
struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::unique_ptr<ExprAST> defineValue;
};

// FUNCDEF := 'func' IDENT '(' (IDENT TYPE (IDENT TYPE)*)? ')' (':' TYPE)? EXPR ENDLINE
// TODO: | func add i64 -> i64 -> i64 = a -> b -> a+b
struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    // std::vector<std::unique_ptr<AST>> funcBody;
    std::unique_ptr<ExprAST> returnValue;
};

// TOP := (EXPR | DEF | ASSIGN) ENDLINE TOP | ()
struct TopLevelAST : public AST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> statements;
};

}