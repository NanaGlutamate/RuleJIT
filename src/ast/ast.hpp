#pragma once

// member pointer of AST to AST is permitted not to be nullptr; type pointer to nullptr means auto type

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ast/astvisitor.hpp"
#include "ast/type.hpp"
#include "defines/typedef.hpp"

using ASTTokenType = std::string;

#define ACCEPT_FUNCTION                                                                                                \
    void accept(ASTVisitor *v) override { v->visit(*this); }

namespace rulejit {

struct AST {
    virtual void accept(ASTVisitor *) = 0;
    virtual ~AST() = default;
};

template <typename T> bool isType(AST *ast) { return dynamic_cast<T *>(ast) != nullptr; }

template <typename T, typename Src> std::unique_ptr<T> asType(Src ast) { return std::unique_ptr<T>(ast.release()); }

// EXPR := BINOP | LEXPR | LITERAL | FUNCCALL | COMPLEX | BRANCH | LOOP | BLOCK | '(' EXPR ')'
struct ExprAST : public AST {
    std::unique_ptr<TypeInfo> type;
    ExprAST(std::unique_ptr<TypeInfo> type) : type(std::move(type)) {}
};

template <typename T> T* isType(std::unique_ptr<ExprAST>& ast) { return dynamic_cast<T *>(ast.get()); }

// // LEXPR := IDENT | MEMBER
// struct AssignableExprAST : public ExprAST {
//     AssignableExprAST(std::unique_ptr<TypeInfo> type) : ExprAST(std::move(type)) {}
// };

// IDENT
// x
struct IdentifierExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    ASTTokenType name;
    template <typename S>
    IdentifierExprAST(std::unique_ptr<TypeInfo> type, S &&name)
        : ExprAST(std::move(type)), name(std::forward<S>(name)) {}
    template <typename S> IdentifierExprAST(S &&name) : IdentifierExprAST(nullptr, std::forward<S>(name)) {}
};

// MEMBER := EXPR '.' IDENT | EXPR '[' EXPR ']'
// vector.x (equals to vector["x"](literal string only)) | make_vector(1, 2, 3).x
struct MemberAccessExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> baseVar;
    // TODO: x[1, 2, 3]
    std::unique_ptr<ExprAST> memberToken;
    MemberAccessExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> baseVar,
                        std::unique_ptr<ExprAST> memberToken)
        : ExprAST(std::move(type)), baseVar(std::move(baseVar)), memberToken(std::move(memberToken)) {}
    MemberAccessExprAST(std::unique_ptr<ExprAST> baseVar, std::unique_ptr<ExprAST> memberToken)
        : MemberAccessExprAST(nullptr, std::move(baseVar), std::move(memberToken)) {}
};

// // ARRAYINDEX := EXPR '[' (EXPR (',' EXPR)*)? ']'
// // type vector{x int;y int;z int};vector["x"] shall pass?
// struct ArrayIndexExprAST : public AssignableExprAST {
//     ACCEPT_FUNCTION;
//     std::unique_ptr<ExprAST> baseVar;
//     std::unique_ptr<ExprAST> index;
//     ArrayIndexExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> baseVar, std::unique_ptr<ExprAST>
//     index)
//         : AssignableExprAST(std::move(type)), baseVar(std::move(baseVar)), index(std::move(index)) {}
//     ArrayIndexExprAST(std::unique_ptr<ExprAST> baseVar, std::unique_ptr<ExprAST> index)
//         : ArrayIndexExprAST(nullptr, std::move(baseVar), std::move(index)) {}
// };

// LITERAL
// "abc" | 12 | 1e3 TODO: | named literal: constexpr
struct LiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::string value;
    template <typename S>
    LiteralExprAST(std::unique_ptr<TypeInfo> type, S &&value)
        : ExprAST(std::move(type)), value(std::forward<S>(value)) {}
    // template <typename S> LiteralExprAST(S &&value) : LiteralExprAST(nullptr, std::forward<S>(value)) {}
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

struct BinOpExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::string op;
    std::unique_ptr<ExprAST> lhs, rhs;
    BinOpExprAST(std::unique_ptr<TypeInfo> type, std::string op, std::unique_ptr<ExprAST> lhs,
                 std::unique_ptr<ExprAST> rhs)
        : ExprAST(std::move(type)), op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    BinOpExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : BinOpExprAST(nullptr, std::move(op), std::move(lhs), std::move(rhs)) {}
};

struct UnaryOpExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::string op;
    std::unique_ptr<ExprAST> rhs;
    UnaryOpExprAST(std::unique_ptr<TypeInfo> type, std::string op, std::unique_ptr<ExprAST> rhs)
        : ExprAST(std::move(type)), op(std::move(op)), rhs(std::move(rhs)) {}
    UnaryOpExprAST(std::string op, std::unique_ptr<ExprAST> rhs)
        : UnaryOpExprAST(nullptr, std::move(op), std::move(rhs)) {}
};

// BRANCH := 'if' '(' EXPR ')' EXPR 'else' EXPR
// if(a==0){1}else{2} | if(a) 1 else 4
struct BranchExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> trueExpr;
    std::unique_ptr<ExprAST> falseExpr;
    BranchExprAST(std::unique_ptr<TypeInfo> type, std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> trueExpr,
                  std::unique_ptr<ExprAST> falseExpr)
        : ExprAST(std::move(type)), condition(std::move(condition)), trueExpr(std::move(trueExpr)),
          falseExpr(std::move(falseExpr)) {}
    BranchExprAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> trueExpr,
                  std::unique_ptr<ExprAST> falseExpr)
        : BranchExprAST(nullptr, std::move(condition), std::move(trueExpr), std::move(falseExpr)) {}
};

// TODO:
// COMPLEX := (SLICETYPE | ARRAYTYPE | IDENT) '{' (EXPR (',' EXPR)*)? '}' | IDENT '{' (IDENT ':' EXPR (',' IDENT ':'
// EXPR)*)?
// '}'
// []i64{1, 2, 3} | Info{base: Base{.name: "abc", .value: 3}, .time: 13} | Vector3{.x: 1, .y: 2, .z: 3}
struct ComplexLiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::tuple<std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>>> members;
    template <typename V>
    ComplexLiteralExprAST(std::unique_ptr<TypeInfo> type, V &&members)
        : ExprAST(std::move(type)), members(std::forward<V>(members)) {}
    template <typename V>
    ComplexLiteralExprAST(V &&members) : ComplexLiteralExprAST(nullptr, std::forward<V>(members)) {}
};

// LOOP := 'while' '(' EXPR ')' EXPR
// while(x!=0){x+=1;x;}
// {init;while(condition){body})}, init and body must have same type
struct LoopAST : public ExprAST {
    ACCEPT_FUNCTION;
    ASTTokenType label;
    std::unique_ptr<ExprAST> init;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> body;
    template <typename S>
    LoopAST(std::unique_ptr<TypeInfo> type, S &&label, std::unique_ptr<ExprAST> init,
            std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> body)
        : ExprAST(std::move(type)), label(std::forward<S>(label)), init(std::move(init)),
          condition(std::move(condition)), body(std::move(body)) {}
    template <typename S>
    LoopAST(S &&label, std::unique_ptr<ExprAST> init, std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> body)
        : LoopAST(nullptr, std::forward<S>(label), std::move(init), std::move(condition), std::move(body)) {}
    LoopAST(std::unique_ptr<ExprAST> init, std::unique_ptr<ExprAST> condition, std::unique_ptr<ExprAST> body)
        : LoopAST(nullptr, "", std::move(init), std::move(condition), std::move(body)) {}
};

// BLOCK := '{' ((EXPR | DEF | ASSIGN) ENDLINE)* EXPR ENDLINE? '}'
// {var x i64 = 12; x;}
struct BlockExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<ExprAST>> exprs;
    // std::unique_ptr<ExprAST> value;
    BlockExprAST(std::unique_ptr<TypeInfo> type, std::vector<std::unique_ptr<ExprAST>> &&exprs)
        : ExprAST(std::move(type)), exprs(std::move(exprs)) {}
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> &&exprs) : BlockExprAST(nullptr, std::move(exprs)) {}
};

// // ASSIGN := LEXPR '=' EXPR
// // x = 12 CAUTION: no returns and not an expression
// struct AssignmentAST : public AST {
//     ACCEPT_FUNCTION;
//     std::unique_ptr<AssignableExprAST> target;
//     std::unique_ptr<ExprAST> value;
//     AssignmentAST(std::unique_ptr<AssignableExprAST> target, std::unique_ptr<ExprAST> value)
//         : target(std::move(target)), value(std::move(value)) {}
// };

struct NoReturnExprAST : public ExprAST {
    NoReturnExprAST() : ExprAST(std::make_unique<TypeInfo>(NoInstanceType)) {}
};

struct ControlFlowAST : public NoReturnExprAST {
    ACCEPT_FUNCTION;
    enum class ControlFlowType {
        BREAK,
        CONTINUE,
        RETURN,
    } controlFlowType;
    ASTTokenType label;
    // never nullptr, use nop instead
    std::unique_ptr<ExprAST> value;
    template <typename S>
    ControlFlowAST(ControlFlowType controlFlowType, S &&label, std::unique_ptr<ExprAST> value)
        : controlFlowType(controlFlowType), label(std::forward<S>(label)), value(std::move(value)) {}
};

// DEF := TYPEDEF | VARDEF | FUNCDEF
struct DefAST : public NoReturnExprAST {
    ASTTokenType name;
    template <typename S> DefAST(S &&name) : name(std::forward<S>(name)), NoReturnExprAST() {}
};

// TYPEDEF := 'type' IDENT TYPE ('|' TYPE)*
// type Vector3 struct {x f64; y f64; z f64;} | type double f64 | TODO: type Reference<T> class {T item;}
struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> definedType;
    enum class TypeDefType {
        NORMAL,
        ALIAS,
    } typeDefType;
    template <typename S>
    TypeDefAST(S &&name, std::unique_ptr<TypeInfo> definedType, TypeDefType typeDefType = TypeDefType::NORMAL)
        : DefAST(std::forward<S>(name)), definedType(std::move(definedType)), typeDefType(typeDefType) {}
};

// VARDEF := 'var' IDENT TYPE ('='? EXPR)? TODO: | 'var' IDENT ':=' EXPR
// var x []i64 = []i64{1, 3, 4};
// TODO: var x []i64 = {1, 3, 4};
// TODO: var x [4]f64;
// TODO: var x [-]i64 {1, 3, 4};
// TODO: var x []i64 {1, 3, 4};
struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> valueType;
    std::unique_ptr<ExprAST> definedValue;
    enum class VarDefType {
        NORMAL,
        AUTO,
    } varDefType;
    template <typename S>
    VarDefAST(S &&name, std::unique_ptr<TypeInfo> valueType, std::unique_ptr<ExprAST> definedValue,
              VarDefType varDefType = VarDefType::NORMAL)
        : DefAST(std::forward<S>(name)), valueType(std::move(valueType)), definedValue(std::move(definedValue)),
          varDefType(varDefType) {}
};

// FUNCDEF := 'func' IDENT '(' (IDENT? TYPE (',' IDENT? TYPE)*)? ')' ('->' TYPE)? ':' EXPR | 'extern' IDENT '(' (IDENT?
// TYPE (',' IDENT? TYPE)*)? ')' ('->' TYPE)?
// TODO: return lambda
struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> funcType;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::unique_ptr<ExprAST> returnValue;
    std::unique_ptr<TypeInfo> captures;
    enum class FuncDefType {
        NORMAL,
        MEMBER,
        SYMBOLIC,
        LAMBDA,
    } funcDefType;
    template <typename S, typename V1>
    FunctionDefAST(S &&name, std::unique_ptr<TypeInfo> funcType, V1 &&params, std::unique_ptr<ExprAST> returnValue,
                   FuncDefType funcDefType = FuncDefType::NORMAL)
        : DefAST(std::forward<S>(name)), funcType(std::move(funcType)), params(std::forward<V1>(params)),
          returnValue(std::move(returnValue)), funcDefType(funcDefType) {
        captures = nullptr;
    }
};

inline decltype(auto) nop() { return std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(NoInstanceType), ""); }

// symbol table operations
// EXTERN := 'extern' real_extern_token/*for extern type, is underlying type in script*/ ('as' used_token
// token_type_for_type_check)?
// extern type u64 as Vector3 struct {x f64; y f64; z f64;}
// extern func vadd(u64, u64):u64 as +(Vector3, Vector3):Vector3
// extern func memberAccess(u64, u64):f64 as .(Vector3, const string):f64
struct SymbolDefAST : public DefAST {
    ACCEPT_FUNCTION;
    enum class SymbolCommandType {
        IMPORT,
        EXPORT,
        EXTERN,
    } symbolCommandType;
    // TODO: alias name
    std::unique_ptr<TypeInfo> definedType;
    template <typename S>
    SymbolDefAST(S &&name, SymbolCommandType symbolCommandType, std::unique_ptr<TypeInfo> definedType)
        : DefAST(std::forward<S>(name)), symbolCommandType(symbolCommandType), definedType(std::move(definedType)) {}
};

} // namespace rulejit