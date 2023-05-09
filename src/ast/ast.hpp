/**
 * @file ast.hpp
 * @author djw
 * @brief AST/AST
 * @date 2023-03-27
 *
 * @details Includes AST node defines
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-04-20</td><td>Template.</td></tr>
 * <tr><td>djw</td><td>2023-04-23</td><td>Add lambda support.</td></tr>
 * </table>
 */
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
#include "tools/pointercast.hpp"

using ASTTokenType = std::string;

#define ACCEPT_FUNCTION                                                                                                \
    void accept(ASTVisitor *v) override { v->visit(*this); }

namespace rulejit {

/**
 * @brief pure virtual base class for all AST node
 *
 */
struct IAST {
    virtual void accept(ASTVisitor *) = 0;
    virtual ~IAST() = default;
};

/**
 * @brief check if AST node can dynamic_cast to given AST type
 *
 * @tparam T target AST type
 * @param ast pointer to AST need checked
 * @return bool
 */
template <typename T> bool isType(IAST *ast) { return dynamic_cast<T *>(ast) != nullptr; }

// /**
//  * @brief dynamic_cast through unique_ptr holds AST node
//  *
//  * @tparam T target AST type
//  * @tparam Src source unique pointer type
//  * @param ast source pointer
//  * @return std::unique_ptr<T> target unique pointer type
//  */
// template <typename T, typename Src> std::unique_ptr<T> asType(Src ast) { return std::unique_ptr<T>(ast.release()); }

// EXPR := BINOP | LEXPR | LITERAL | FUNCCALL | COMPLEX | BRANCH | LOOP | BLOCK | '(' EXPR ')'
/**
 * @brief indicates Expression AST
 *
 */
struct ExprAST : public IAST {
    std::unique_ptr<TypeInfo> type;
    virtual std::unique_ptr<ExprAST> copy() = 0;
    ExprAST(std::unique_ptr<TypeInfo> type) : type(std::move(type)) {}
};

/**
 * @brief check if AST node can dynamic_cast to given AST type
 *
 * @tparam T target AST type
 * @param ast unique pointer to AST need checked
 * @return T*, reuse for visit ast as AST type T
 */
template <typename T> T *isType(std::unique_ptr<ExprAST> &ast) { return dynamic_cast<T *>(ast.get()); }

// IDENT
// x
/**
 * @brief indicates Identifier
 *
 */
struct IdentifierExprAST final : public ExprAST {
    ACCEPT_FUNCTION;
    ASTTokenType name;

    template <typename S>
    IdentifierExprAST(std::unique_ptr<TypeInfo> type, S &&name)
        : ExprAST(std::move(type)), name(std::forward<S>(name)) {}
    template <typename S> IdentifierExprAST(S &&name) : IdentifierExprAST(nullptr, std::forward<S>(name)) {}
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<IdentifierExprAST>(name);
        }
        return std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>(*type), name);
    }
};

// MEMBER := EXPR '.' IDENT | EXPR '[' EXPR ']'
// vector.x (equals to vector["x"](literal string only)) | make_vector(1, 2, 3).x
/**
 * @brief indicated member access expression
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<MemberAccessExprAST>(baseVar->copy(), memberToken->copy());
        }
        return std::make_unique<MemberAccessExprAST>(std::make_unique<TypeInfo>(*type), baseVar->copy(),
                                                     memberToken->copy());
    }
};

// LITERAL
// "abc" | 12 | 1e3 TODO: | named literal: constexpr
/**
 * @brief indicates string, int and real literal
 *
 */
struct LiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::string value;

    template <typename S>
    LiteralExprAST(std::unique_ptr<TypeInfo> type, S &&value)
        : ExprAST(std::move(type)), value(std::forward<S>(value)) {}
    std::unique_ptr<ExprAST> copy() override {
        return std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(*type), value);
    }
};

// FUNCCALL := IDENT '(' (EXPR (',' EXPR)*)? ')'
// TODO: | EXPR INFIX EXPR
// TODO: | add(1)(2)
// add(1+3)
/**
 * @brief indicates function call expression
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        std::vector<std::unique_ptr<ExprAST>> newParams;
        for (auto &param : params) {
            newParams.push_back(param->copy());
        }
        if (!type) {
            return std::make_unique<FunctionCallExprAST>(functionIdent->copy(), std::move(newParams));
        }
        return std::make_unique<FunctionCallExprAST>(std::make_unique<TypeInfo>(*type), functionIdent->copy(),
                                                     std::move(newParams));
    }
};

/**
 * @brief indicates binary op expression
 *
 */
struct BinOpExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    ASTTokenType op;
    std::unique_ptr<ExprAST> lhs, rhs;

    template <typename S>
    BinOpExprAST(std::unique_ptr<TypeInfo> type, S &&op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : ExprAST(std::move(type)), op(std::forward<S>(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    template <typename S>
    BinOpExprAST(S &&op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : BinOpExprAST(nullptr, std::forward<S>(op), std::move(lhs), std::move(rhs)) {}
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<BinOpExprAST>(op, lhs->copy(), rhs->copy());
        }
        return std::make_unique<BinOpExprAST>(std::make_unique<TypeInfo>(*type), op, lhs->copy(), rhs->copy());
    }
};

/**
 * @brief indicates unary op expression
 *
 */
struct UnaryOpExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    ASTTokenType op;
    std::unique_ptr<ExprAST> rhs;

    template <typename S>
    UnaryOpExprAST(std::unique_ptr<TypeInfo> type, S &&op, std::unique_ptr<ExprAST> rhs)
        : ExprAST(std::move(type)), op(std::forward<S>(op)), rhs(std::move(rhs)) {}
    template <typename S>
    UnaryOpExprAST(S &&op, std::unique_ptr<ExprAST> rhs)
        : UnaryOpExprAST(nullptr, std::forward<S>(op), std::move(rhs)) {}
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<UnaryOpExprAST>(op, rhs->copy());
        }
        return std::make_unique<UnaryOpExprAST>(std::make_unique<TypeInfo>(*type), op, rhs->copy());
    }
};

// BRANCH := 'if' '(' EXPR ')' EXPR 'else' EXPR
// if(a==0){1}else{2} | if(a) 1 else 4
/**
 * @brief indicates branch expression like "if(*condition*) *expr1* else *expr2*"
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<BranchExprAST>(condition->copy(), trueExpr->copy(), falseExpr->copy());
        }
        return std::make_unique<BranchExprAST>(std::make_unique<TypeInfo>(*type), condition->copy(), trueExpr->copy(),
                                               falseExpr->copy());
    }
};

// TODO:
// COMPLEX := (SLICETYPE | ARRAYTYPE | IDENT) '{' (EXPR (',' EXPR)*)? '}' | IDENT '{' (IDENT ':' EXPR (',' IDENT ':'
// EXPR)*)?
// '}'
// []i64{1, 2, 3} | Info{base: Base{.name: "abc", .value: 3}, .time: 13} | Vector3{.x: 1, .y: 2, .z: 3}
/**
 * @brief indicates complex literal like "Vector3{.x: 1, .y: 2, .z: 3}"
 *
 */
struct ComplexLiteralExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::tuple<std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>>> members;

    template <typename V>
    ComplexLiteralExprAST(std::unique_ptr<TypeInfo> type, V &&members)
        : ExprAST(std::move(type)), members(std::forward<V>(members)) {}
    std::unique_ptr<ExprAST> copy() override {
        std::vector<std::tuple<std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>>> newMembers;
        for (auto &member : members) {
            newMembers.emplace_back(std::get<0>(member)->copy(), std::get<1>(member)->copy());
        }
        return std::make_unique<ComplexLiteralExprAST>(std::make_unique<TypeInfo>(*type), std::move(newMembers));
    }
};

// LOOP := 'while' '(' EXPR ')' EXPR
// while(x!=0){x+=1;x;}
// {init;while(condition){body})}
/**
 * @brief indicates loop statement like "while(*condition*) *expr*", normally no returns
 *
 * @attention if init and body have same type, this expression can return.
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        if (!type) {
            return std::make_unique<LoopAST>(label, init->copy(), condition->copy(), body->copy());
        }
        return std::make_unique<LoopAST>(std::make_unique<TypeInfo>(*type), label, init->copy(), condition->copy(),
                                         body->copy());
    }
};

// BLOCK := '{' ((EXPR | DEF | ASSIGN) ENDLINE)* EXPR ENDLINE? '}'
// {var x i64 = 12; x;}
/**
 * @brief indicates block expression like "{1; 2}"
 *
 */
struct BlockExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<ExprAST>> exprs;

    BlockExprAST(std::unique_ptr<TypeInfo> type, std::vector<std::unique_ptr<ExprAST>> &&exprs)
        : ExprAST(std::move(type)), exprs(std::move(exprs)) {}
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> &&exprs) : BlockExprAST(nullptr, std::move(exprs)) {}
    std::unique_ptr<ExprAST> copy() override {
        std::vector<std::unique_ptr<ExprAST>> newExprs;
        for (auto &expr : exprs) {
            newExprs.emplace_back(expr->copy());
        }
        if (!type) {
            return std::make_unique<BlockExprAST>(std::move(newExprs));
        }
        return std::make_unique<BlockExprAST>(std::make_unique<TypeInfo>(*type), std::move(newExprs));
    }
};

/**
 * @brief base class for expressions with no return
 *
 */
struct NoReturnExprAST : public ExprAST {
    NoReturnExprAST() : ExprAST(std::make_unique<TypeInfo>(NoInstanceType)) {}
};

/**
 * @brief indicates control flow command like continue, break or return
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        return std::make_unique<ControlFlowAST>(controlFlowType, label, value->copy());
    }
};

// DEF := TYPEDEF | VARDEF | FUNCDEF
/**
 * @brief base class for definition AST
 *
 */
struct DefAST : public NoReturnExprAST {
    ASTTokenType name;
    template <typename S> DefAST(S &&name) : name(std::forward<S>(name)), NoReturnExprAST() {}
};

// TYPEDEF := 'type' IDENT TYPE ('|' TYPE)*
// type Vector3 struct {x f64; y f64; z f64;} | type double f64 | TODO: type Reference<T> class {T item;}
/**
 * @brief indicates type defines
 *
 */
struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::vector<std::tuple<std::string, TypeInfo>> definedType;
    enum class TypeDefType {
        NORMAL,
        // CLASSTYPE,
        ALIAS,
    } typeDefType;

    // TODO: typeclass support? or interface like go?
    template <typename V, typename S>
    TypeDefAST(S &&name, V&& definedType, TypeDefType typeDefType = TypeDefType::NORMAL)
        : DefAST(std::forward<S>(name)), definedType(std::forward<V>(definedType)), typeDefType(typeDefType) {}
    std::unique_ptr<ExprAST> copy() override {
        return std::make_unique<TypeDefAST>(name, definedType, typeDefType);
    }
};

// VARDEF := 'var' IDENT TYPE ('='? EXPR)? TODO: | 'var' IDENT ':=' EXPR
// var x []i64 = []i64{1, 3, 4};
// TODO: var x []i64 = {1, 3, 4};
// TODO: var x [4]f64;
// TODO: var x [-]i64 {1, 3, 4};
/**
 * @brief indicates variable defines
 *
 */
struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> valueType;
    std::unique_ptr<ExprAST> definedValue;
    enum class VarDefType {
        NORMAL,
        CONSTANT,
    } varDefType;

    template <typename S>
    VarDefAST(S &&name, std::unique_ptr<TypeInfo> valueType, std::unique_ptr<ExprAST> definedValue,
              VarDefType varDefType = VarDefType::NORMAL)
        : DefAST(std::forward<S>(name)), valueType(std::move(valueType)), definedValue(std::move(definedValue)),
          varDefType(varDefType) {}
    std::unique_ptr<ExprAST> copy() override {
        return std::make_unique<VarDefAST>(name, std::make_unique<TypeInfo>(*valueType), definedValue->copy(),
                                           varDefType);
    }
};

// FUNCDEF := 'func' IDENT '(' (IDENT TYPE (',' IDENT TYPE)*)? ')' ('->' TYPE)? EXPR
// TODO: return lambda
/**
 * @brief indicates function defines
 *
 */
struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<TypeInfo> funcType;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::vector<std::unique_ptr<IdentifierExprAST>> captures;
    std::unique_ptr<ExprAST> returnValue;
    enum class FuncDefType {
        NORMAL,
        MEMBER,
        SYMBOLIC,
    } funcDefType;

    template <typename S, typename V1>
    FunctionDefAST(S &&name, std::unique_ptr<TypeInfo> funcType, V1 &&params, std::unique_ptr<ExprAST> returnValue,
                   FuncDefType funcDefType = FuncDefType::NORMAL)
        : DefAST(std::forward<S>(name)), funcType(std::move(funcType)), params(std::forward<V1>(params)), captures(),
          returnValue(std::move(returnValue)), funcDefType(funcDefType) {}
    std::unique_ptr<ExprAST> copy() override {
        std::vector<std::unique_ptr<IdentifierExprAST>> newParams;
        for (auto &param : params) {
            newParams.push_back(
                std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>(*(param->type)), param->name));
        }
        return std::make_unique<FunctionDefAST>(name, std::make_unique<TypeInfo>(*funcType), std::move(newParams),
                                                returnValue->copy(), funcDefType);
    }
};

// symbol table operations
// EXTERN := 'extern' real_extern_token/*for extern type, is underlying type in script*/ ('as' used_token
// token_type_for_type_check)?
// extern type u64 as Vector3 struct {x f64; y f64; z f64;}
// extern func vadd(u64, u64):u64 as +(Vector3, Vector3):Vector3
// extern func memberAccess(u64, u64):f64 as .(Vector3, const string):f64
/**
 * @brief indicates symbol definition statements like "extern sin(f64):f64"
 *
 */
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
    std::unique_ptr<ExprAST> copy() override {
        return std::make_unique<SymbolDefAST>(name, symbolCommandType, std::make_unique<TypeInfo>(*definedType));
    }
};

// TODO: template support, will not execute or generate code
// TEMPLATE := 'func' '<' IDENT (',' IDENT)* '>'
struct TemplateDefAST : public NoReturnExprAST {
    ACCEPT_FUNCTION;
    std::vector<ASTTokenType> tparams;
    std::unique_ptr<DefAST> def;

    template <typename V1>
    TemplateDefAST(V1 &&tparams, std::unique_ptr<DefAST> def)
        : NoReturnExprAST(), tparams(std::forward<V1>(tparams)), def(std::move(def)) {}
    std::unique_ptr<ExprAST> copy() override {
        auto tmp = def->copy();
        std::unique_ptr<DefAST> newDef = tools::myunique::unique_cast<DefAST>(tmp);
        return std::make_unique<TemplateDefAST>(tparams, std::move(newDef));
    }
};

struct ClosureExprAST : public ExprAST {
    ACCEPT_FUNCTION;
    bool explicitCapture;
    std::vector<std::unique_ptr<IdentifierExprAST>> captures;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::unique_ptr<ExprAST> returnValue;

    ClosureExprAST(std::unique_ptr<TypeInfo> &&type, bool explicitCapture,
                   std::vector<std::unique_ptr<IdentifierExprAST>> &&captures,
                   std::vector<std::unique_ptr<IdentifierExprAST>> &&params, std::unique_ptr<ExprAST> &&returnValue)
        : ExprAST(std::move(type)), explicitCapture(explicitCapture), captures(std::move(captures)),
          params(std::move(params)), returnValue(std::move(returnValue)) {}
    std::unique_ptr<ExprAST> copy() override {
        std::vector<std::unique_ptr<IdentifierExprAST>> newCaptures;
        for (auto &capture : captures) {
            newCaptures.push_back(
                std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>(*(capture->type)), capture->name));
        }
        std::vector<std::unique_ptr<IdentifierExprAST>> newParams;
        for (auto &param : params) {
            newParams.push_back(
                std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>(*(param->type)), param->name));
        }
        return std::make_unique<ClosureExprAST>(std::make_unique<TypeInfo>(*type), explicitCapture,
                                                std::move(newCaptures), std::move(newParams), returnValue->copy());
    }
};

// struct TemplateInstantiationIdentifierExprAST : public ExprAST {};

// struct TraitDefAST : public NoReturnExprAST {};

// struct TypeMatchExprAST : public ExprAST {};

// struct TypeCastExprAST : public ExprAST {};

// // func <T> zero():T -> match T {fit []V => []V{}; is i64 => 0; else => null;}
// TODO: use fit<V> []V to avoid call to freeMatch()?
// struct TypeMatchExprAST : public ExprAST {
//     // ACCEPT_FUNCTION;
//     std::unique_ptr<ExprAST> expr;
//     std::vector<std::unique_ptr<ExprAST>> matchExprs;
//     std::vector<std::unique_ptr<ExprAST>> matchValues;
// };

/**
 * @brief tool function to instantly get a literal expression with type NoInstanceType
 *
 * @return unique pointer to Literal expression AST which type is NoInstanceType
 */
inline auto nop() { return std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(NoInstanceType), ""); }

} // namespace rulejit