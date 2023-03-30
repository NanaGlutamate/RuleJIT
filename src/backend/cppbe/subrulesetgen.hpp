/**
 * @file subrulesetgen.hpp
 * @author djw
 * @brief CQ/CPPBE/SubRuleSet code generator
 * @date 2023-03-27
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <algorithm>
#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "backend/cppbe/metainfo.hpp"
#include "backend/cppbe/template.hpp"
#include "frontend/ruleset/rulesetparser.h"
#include "tools/myassert.hpp"

namespace rulejit::cppgen {

struct SubRuleSetCodeGen : public ASTVisitor {
    SubRuleSetCodeGen(ContextStack &context, rulesetxml::RuleSetMetaInfo &metaInfo) : c(context), m(metaInfo){};
    std::string friend operator|(std::unique_ptr<ExprAST> &e, SubRuleSetCodeGen &t) {
        // t.isSubRuleSet = true;
        t.loadedVar.clear();
        t.returned.clear();
        t.tmp = 0;
        e->accept(&t);
        return std::move(t.returned);
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        // only thing differs from common cppcodegen
        if (std::find(m.inputVar.begin(), m.inputVar.end(), v.name) != m.inputVar.end()) {
            returned += std::format("(_in.{})", v.name);
        } else if (std::find(m.outputVar.begin(), m.outputVar.end(), v.name) != m.outputVar.end()) {
            returned += std::format("(_out.{})", v.name);
        } else if (std::find(m.cacheVar.begin(), m.cacheVar.end(), v.name) != m.cacheVar.end()) {
            // cannot remove loadCache, cause assign will evaluate rhs befor lhs
            // TODO: name string & member pointer -> index & boost::pfr

            // if (!loadedVar.contains(v.name)) {
            auto cnt = std::find(m.cacheVar.begin(), m.cacheVar.end(), v.name) - m.cacheVar.begin();
            returned += std::format("(loadCache(_base, &_Cache::{0}, {1}), cache.{0})", v.name, cnt);
            //     loadedVar.emplace(v.name);
            // }else{
            //     returned += std::format("(cache.{})", v.name);
            // }
        } else {
            returned += std::format("({})", v.name);
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        if (auto p = dynamic_cast<LiteralExprAST *>(v.memberToken.get()); p && *(p->type) == StringType) {
            returned += "(";
            v.baseVar->accept(this);
            returned += std::format(".{})", p->value);
        } else {
            returned += "(";
            v.baseVar->accept(this);
            returned += "[";
            v.memberToken->accept(this);
            returned += "])";
        }
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (v.type->isFunctionType()) {
            returned += v.value;
            return;
        }
        returned += "(";
        if (*(v.type) == RealType) {
            returned += v.value.data();
        } else if (*(v.type) == NoInstanceType) {
            returned += "NoInstanceType{}";
        } else if (*(v.type) == StringType) {
            returned += "R\"(" + v.value + ")\"";
        } else {
            return setError("Unknown literal type: " + v.type->toString());
        }
        returned += ")";
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        returned += "(";
        v.functionIdent->accept(this);
        returned += "(";
        bool init = true;
        for (auto &&arg : v.params) {
            if (init) {
                init = false;
            } else {
                returned += ", ";
            }
            arg->accept(this);
        }
        returned += "))";
    }
    VISIT_FUNCTION(BinOpExprAST) {
        if (v.op != "=") {
            static const std::unordered_map<std::string, std::string> opTrans{
                {"or", "||"},
                {"and", "&&"},
                {"xor", "^"},
            };
            returned += "(double(";
            v.lhs->accept(this);
            auto tmp = v.op;
            if (auto it = opTrans.find(tmp); it != opTrans.end()) {
                tmp = it->second;
            }
            returned += ") " + tmp + " double(";
            v.rhs->accept(this);
            returned += "))";
        } else {
            v.lhs->accept(this);
            returned += " = double(";
            v.rhs->accept(this);
            returned += ")";
        }
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        returned += "(";
        if (v.op == "-") {
            returned += "-";
        } else if (v.op == "not" || v.op == "!") {
            returned += "!";
        } else {
            return setError("Unknown unary op: " + v.op);
        }
        v.rhs->accept(this);
        returned += ")";
    }
    VISIT_FUNCTION(BranchExprAST) {
        if (*(v.type) != NoInstanceType) {
            returned += "(";
            v.condition->accept(this);
            returned += " ? ";
            v.trueExpr->accept(this);
            returned += " : ";
            v.falseExpr->accept(this);
            returned += ")";
        } else {
            returned += "if";
            v.condition->accept(this);
            returned += "{";
            v.trueExpr->accept(this);
            returned += ";}else{";
            v.falseExpr->accept(this);
            returned += ";}";
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        returned += "(" + CppStyleType(*(v.type)) + "{";
        if (v.type->isArrayType()) {
            for (auto &[ident, value] : v.members) {
                value->accept(this);
                returned += ", ";
            }
        } else if (v.type->isBaseType()) {
            auto it = c.global.typeDef.find(v.type->toString());
            if (it == c.global.typeDef.end()) {
                return setError("Unknown type: " + v.type->toString());
            }
            auto &def = it->second;
            std::unordered_map<std::string, ExprAST *> tmp;
            for (auto &[ident, value] : v.members) {
                tmp.emplace(dynamic_cast<LiteralExprAST *>(ident.get())->value, value.get());
            }
            auto tokens = def.getTokens();
            for (size_t i = 0; i < tokens.size(); ++i) {
                auto it = tmp.find(tokens[i]);
                if (it == tmp.end()) {
                    returned += "{}";
                } else {
                    it->second->accept(this);
                }
                returned += ", ";
            }
        } else {
            return setError("Unknown type: " + v.type->toString());
        }
        returned += "})";
    }
    VISIT_FUNCTION(LoopAST) {
        if (*(v.type) != NoInstanceType) {
            return setError("loop with returned value not supported");
        }
        // only support while
        returned += "while(";
        v.condition->accept(this);
        returned += "){";
        v.body->accept(this);
        returned += ";}";
    }
    VISIT_FUNCTION(BlockExprAST) {
        if (*(v.type) == NoInstanceType) {
            returned += "{";
            for (auto &&expr : v.exprs) {
                expr->accept(this);
                returned += ";";
            }
            returned += "}";
        } else if (v.exprs.size() == 1) {
            // returned += "(";
            v.exprs[0]->accept(this);
            // returned += ")";
        } else {
            returned += "([&](){";
            if (v.exprs.size() != 0) {
                for (size_t i = 0; i < v.exprs.size() - 1; ++i) {
                    v.exprs[i]->accept(this);
                    returned += ";";
                }
                returned += "return ";
                v.exprs.back()->accept(this);
                returned += ";";
            }
            returned += "}())";
        }
    }
    VISIT_FUNCTION(ControlFlowAST) { return setError("ControlFlowAST not supported"); }
    VISIT_FUNCTION(TypeDefAST) { return setError("TypeDefAST not supported"); }
    VISIT_FUNCTION(VarDefAST) {
        if (std::find(m.inputVar.begin(), m.inputVar.end(), v.name) != m.inputVar.end() ||
            std::find(m.outputVar.begin(), m.outputVar.end(), v.name) != m.outputVar.end() ||
            std::find(m.cacheVar.begin(), m.cacheVar.end(), v.name) != m.cacheVar.end()) {
            return setError("Variable redefined(already a input/output/cache): " + v.name);
        }
        if (*(v.valueType) == RealType) {
            returned += "double ";
        } else if (*(v.valueType) == IntType) {
            returned += "size_t ";
        } else {
            returned += "auto ";
        }
        returned += v.name + " = ";
        v.definedValue->accept(this);
    }
    VISIT_FUNCTION(FunctionDefAST) { return setError("FunctionDefAST not supported"); }
    VISIT_FUNCTION(SymbolDefAST) { return setError("SymbolDefAST not supported"); }

  private:
    // bool isSubRuleSet;
    std::set<std::string> loadedVar;
    std::string returned;
    std::size_t tmp;
    std::string getTmpVarName() {
        std::string name = std::format("_tmp{}", tmp++);
        while (std::get<0>(c.seekVarDef(name))) {
            name = std::format("__tmp{}", tmp++);
        }
        return name;
    }
    std::string CppStyleType(const TypeInfo &type) {
        // only support vector and base type
        if (type.isBaseType()) {
            // TODO: typedReal?
            return type.getBaseTypeString();
        }
        if (type.isArrayType()) {
            return "std::vector<" + CppStyleType(type.getElementType()) + ">";
        }
        setError(std::format("unsupported type: {}", type.toString()));
    }
    SET_ERROR_MEMBER("CPP Code Generation", void)
    ContextStack &c;
    rulesetxml::RuleSetMetaInfo &m;
};

} // namespace rulejit::cppgen