#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ast/astvisitor.hpp"
#include "ast/type.hpp"

#define ACCEPT_FUNCTION                                                                                                \
    void accept(ASTVisitor *v) override { v->visit(*this); }

namespace rulejit {

struct AST {
    virtual void accept(ASTVisitor *) = 0;
    virtual ~AST() = default;
};

// EXPR := BINOP | LEXPR | LITERAL | FUNCCALL | COMPLEX | BRANCH | LOOP | BLOCK | '(' EXPR ')'
struct ExprAST : public AST {
    std::unique_ptr<TypeInfo> type;
    ExprAST(std::unique_ptr<TypeInfo> type) : type(std::move(type)) {}
};

// LEXPR := IDENT | MEMBER
struct AssignableExprAST : public ExprAST {
    AssignableExprAST(std::unique_ptr<TypeInfo> type) : ExprAST(std::move(type)) {}
};

// IDENT
// x
struct IdentifierExprAST : public AssignableExprAST {
    ACCEPT_FUNCTION;
    std::string name;
    template <typename S>
    IdentifierExprAST(std::unique_ptr<TypeInfo> type, S &&name)
        : AssignableExprAST(std::move(type)), name(std::forward<S>(name)) {}
    template <typename S> IdentifierExprAST(S &&name) : IdentifierExprAST(nullptr, std::forward<S>(name)) {}
};

// MEMBER := EXPR '.' IDENT | EXPR '[' EXPR ']'
// vector.x (same as vector["x"]) | list[a+b] | make_vector(1, 2, 3).x
struct MemberAccessExprAST : public AssignableExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> baseVar;
    std::unique_ptr<ExprAST> memberToken;
    MemberAccessExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> baseVar,
                        std::unique_ptr<ExprAST> memberToken)
        : AssignableExprAST(std::move(type)), baseVar(std::move(baseVar)), memberToken(std::move(memberToken)) {}
    MemberAccessExprAST(std::unique_ptr<ExprAST> baseVar, std::unique_ptr<ExprAST> memberToken)
        : MemberAccessExprAST(nullptr, std::move(baseVar), std::move(memberToken)) {}
};

// LITERAL
// "abc" | 12 | 1e3 TODO: | constexpr
struct LiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::string value;
    template <typename S>
    LiteralExprAST(std::unique_ptr<TypeInfo> type, S &&value)
        : ExprAST(std::move(type)), value(std::forward<S>(value)) {}
    template <typename S> LiteralExprAST(S &&value) : LiteralExprAST(nullptr, std::forward<S>(value)) {}
};

// FUNCCALL := IDENT '(' (EXPR (',' EXPR)*)? ')'
// TODO: | EXPR INFIX EXPR
// TODO: | add(1)(2)
// add(1+3)
struct FunctionCallExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> functionIdent;
    std::vector<std::unique_ptr<ExprAST>> params;
    template <typename V>
    FunctionCallExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> functionIdent, V &&params)
        : ExprAST(std::move(type)), functionIdent(std::move(functionIdent)), params(std::forward<V>(params)) {}
    template <typename V>
    FunctionCallExprAST(std::unique_ptr<ExprAST> functionIdent, V &&params)
        : FunctionCallExprAST(nullptr, std::move(functionIdent), std::forward<V>(params)) {}
};

// BRANCH := 'if' '(' EXPR ')' EXPR 'else' EXPR
// if(a==0){1}else{2} | if(a) 1 else 4
struct BranchExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> trueBranch;
    std::unique_ptr<ExprAST> falseBranch;
    BranchExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> condition,
                  std::unique_ptr<ExprAST> trueBranch, std::unique_ptr<ExprAST> falseBranch)
        : ExprAST(std::move(type)), condition(std::move(condition)), trueBranch(std::move(trueBranch)),
          falseBranch(std::move(falseBranch)) {}
    BranchExprAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> trueBranch,
                  std::unique_ptr<ExprAST> falseBranch)
        : BranchExprAST(nullptr, std::move(condition), std::move(trueBranch), std::move(falseBranch)) {}
};

// TODO:
// COMPLEX := (SLICETYPE | ARRAYTYPE) '{' (EXPR (',' EXPR)*)? '}' | IDENT '{' (IDENT ':' EXPR (',' IDENT ':' EXPR)*)?
// '}'
// []i64{1, 2, 3} | Info{base: Base{name: "abc", value: 3}, time: 13} | Vector3{x: 1, y: 2, z: 3}
struct ComplexLiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::tuple<std::string, std::unique_ptr<ExprAST>>> members;
    template <typename V>
    ComplexLiteralExprAST(std::unique_ptr<TypeInfo> type, V &&members)
        : ExprAST(std::move(type)), members(std::forward<V>(members)) {}
    template <typename V>
    ComplexLiteralExprAST(V &&members) : ComplexLiteralExprAST(nullptr, std::forward<V>(members)) {}
};

// LOOP := 'while' '(' EXPR ')' EXPR
// while(x!=0){x+=1;x;}
struct LoopAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> body;
    LoopAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> body)
        : ExprAST(std::move(type)), condition(std::move(condition)), body(std::move(body)) {}
    LoopAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> body)
        : LoopAST(nullptr, std::move(condition), std::move(body)) {}
};

// BLOCK := '{' ((EXPR | DEF | ASSIGN) ENDLINE)* EXPR ENDLINE? '}'
// {var x i64 = 12; x;}
struct BlockExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> preStatement;
    std::unique_ptr<ExprAST> value;
    template<typename V>
    BlockExprAST(std::unique_ptr<TypeInfo> type, V &&preStatement, std::unique_ptr<ExprAST> value)
        : ExprAST(std::move(type)), preStatement(std::forward<V>(preStatement)), value(std::move(value)) {}
    template<typename V>
    BlockExprAST(V &&preStatement, std::unique_ptr<ExprAST> value)
        : BlockExprAST(nullptr, std::forward<V>(preStatement), std::move(value)) {}
};

// ASSIGN := LEXPR '=' EXPR
// x = 12 CAUTION: no returns and not an expression
struct AssignmentAST : public AST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> type;
    std::unique_ptr<AssignableExprAST> target;
    std::unique_ptr<ExprAST> value;
    AssignmentAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<AssignableExprAST> target,
                  std::unique_ptr<ExprAST> value)
        : type(std::move(type)), target(std::move(target)), value(std::move(value)) {}  
    AssignmentAST(std::unique_ptr<AssignableExprAST> target, std::unique_ptr<ExprAST> value)
        : AssignmentAST(nullptr, std::move(target), std::move(value)) {}
};

// DEF := TYPEDEF | VARDEF | FUNCDEF
struct DefAST : public AST {
    std::string name;
    template <typename S>
    DefAST(S &&name) : name(std::forward<S>(name)) {}
};

// TYPEDEF := 'type' IDENT TYPE ('|' TYPE)*
// type Vector3 struct {x f64; y f64; z f64;} | type double f64 | TODO: type Reference<T> class {T item;}
struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> type;
    template <typename S>
    TypeDefAST(S &&name, std::unique_ptr<TypeInfo> type) : DefAST(std::forward<S>(name)), type(std::move(type)) {}
};

// VARDEF := 'var' IDENT TYPE ('='? EXPR)? TODO: | 'var' IDENT ':=' EXPR
// var x []i64 = []i64{1, 3, 4};
// TODO: var x []i64 = {1, 3, 4};
// TODO: var x [4]f64;
// TODO: var x [-]i64 {1, 3, 4};
// TODO: var x []i64 {1, 3, 4};
struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> type;
    std::unique_ptr<ExprAST> defineValue;
    template <typename S>
    VarDefAST(S &&name, std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> defineValue)
        : DefAST(std::forward<S>(name)), type(std::move(type)), defineValue(std::move(defineValue)) {}
};

// FUNCDEF := 'func' IDENT '(' (IDENT? TYPE (',' IDENT? TYPE)*)? ')' ('->' TYPE)? ':' EXPR | 'extern' IDENT '(' (IDENT?
// TYPE (',' IDENT? TYPE)*)? ')' ('->' TYPE)?
// TODO: | func add i64 -> i64 -> i64 = a -> b -> a+b
struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> type;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::unique_ptr<ExprAST> returnValue;
    template <typename S, typename V>
    FunctionDefAST(S &&name, std::unique_ptr<TypeInfo> type, V &&params, std::unique_ptr<ExprAST> returnValue)
        : DefAST(std::forward<S>(name)), type(std::move(type)), params(std::forward<V>(params)),
          returnValue(std::move(returnValue)) {}
};

// TODO: INFIX

// TODO: EXTERNFUNC

// TODO: MACRODEF

// TODO:
// var x i32 = 1
//     + 12 //legal?
// TOP := (EXPR | DEF | ASSIGN) ENDLINE TOP | ()
struct TopLevelAST : public AST {
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> statements;
    template <typename V>
    TopLevelAST(V &&statements) : statements(std::forward<V>(statements)) {}
};

} // namespace rulejit