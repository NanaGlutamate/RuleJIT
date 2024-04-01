/**
 * @file pycodegen.hpp
 * @author djw
 * @brief
 * @date 2024-03-11
 *
 * @details DDL-oriented code. not fully supported, not fully tested. need restructure
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2024-03-11</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <algorithm>
#include <expected>
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
#include "tools/stringprocess.hpp"

namespace rulejit::pybe {

struct PYCodeGen;

struct PYTransformCodeGen : public ASTVisitor {
    PYTransformCodeGen() = default;
    struct ParaInfo {
        std::string id;
        double init_value;
    };
    struct ProcessedInfo {
        std::string code;
        std::vector<std::tuple<std::string, std::string>> states;
        std::vector<ParaInfo> paras;
    };
    std::expected<ProcessedInfo, std::errc> gen(std::unique_ptr<ExprAST>& e, PYCodeGen& cg) noexcept {
        err = false;
        topLevel = true;
        returned.clear();
        states.clear();
        paras.clear();
        src = &cg;
        e->accept(this);
        if (err) {
            return std::unexpected(std::errc::invalid_argument);
        }
        return ProcessedInfo{std::move(returned), std::move(states), std::move(paras)};
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST);
    VISIT_FUNCTION(MemberAccessExprAST) {
        if (topLevel) {
            return setError();
        }
        if (auto p = dynamic_cast<LiteralExprAST*>(v.memberToken.get()); p && *(p->type) == StringType) {
            accept(v.baseVar);
            returned = std::format("{}[\"{}\"]", returned, p->value);
        } else {
            accept(v.baseVar);
            auto base = std::move(returned);
            accept(v.memberToken);
            returned = std::format("{}[{}]", base, returned);
        }
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (topLevel) {
            return setError();
        }
        if (v.type->isFunctionType()) {
            returned = v.value;
            return;
        }
        if (*v.type != RealType && *v.type != IntType) {
            return setError();
        }
        returned = v.value;
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        std::string paramList;
        for (auto&& p : v.params) {
            if (!paramList.empty()) {
                paramList.push_back(',');
            }
            accept(p);
            paramList += returned;
        }
        accept(v.functionIdent);
        returned = std::format("{}({})", returned, paramList);
    }
    VISIT_FUNCTION(BinOpExprAST) {
        static std::set<std::string> op_gt{">", ">="};
        static std::set<std::string> op_lt{"<=", "<"};
        static std::set<std::string> op_and{"&&", "and"};
        static std::set<std::string> op_or{"||", "or"};
        static std::set<std::string> op_normal{"+", "-", "*", "/"};
        if (v.op == "=")
            return setError();
        if (topLevel) {
            if (!op_gt.contains(v.op) && !op_lt.contains(v.op) && !op_and.contains(v.op) && !op_or.contains(v.op)) {
                return setError();
            }
            if (op_gt.contains(v.op)) {
                topLevel = false;
                accept(v.lhs);
                auto lhs = std::move(returned);
                accept(v.rhs);
                auto rhs = std::move(returned);
                topLevel = true;
                if (err)
                    return;

                auto p1 = newParaName();
                paras.push_back({p1, 0.0});
                auto p2 = newParaName();
                paras.push_back({p2, 0.0});
                auto index = newIndexName();
                returned = std::format("F(self.state[:,self.{}] * (self.{} + 1.0 / None) + self.{})", index, p1, p2);
                states.emplace_back(std::move(index), std::format("({0} - {1})", lhs, rhs));
            } else if (op_lt.contains(v.op)) {
                topLevel = false;
                accept(v.lhs);
                auto lhs = std::move(returned);
                accept(v.rhs);
                auto rhs = std::move(returned);
                topLevel = true;
                if (err)
                    return;

                auto p1 = newParaName();
                paras.push_back({p1, 0.0});
                auto p2 = newParaName();
                paras.push_back({p2, 0.0});
                auto index = newIndexName();
                returned = std::format("F(self.state[:,self.{}] * (self.{} + 1.0 / None) + self.{})", index, p1, p2);
                states.emplace_back(std::move(index), std::format("({1} - {0})", lhs, rhs));
            } else if (op_and.contains(v.op)) {
                accept(v.lhs);
                auto lhs = std::move(returned);
                accept(v.rhs);
                auto rhs = std::move(returned);
                if (err)
                    return;
                returned = std::format("({} * {})", lhs, rhs);
            } else if (op_or.contains(v.op)) {
                accept(v.lhs);
                auto lhs = std::move(returned);
                accept(v.rhs);
                auto rhs = std::move(returned);
                if (err)
                    return;
                returned = std::format("(1.0 - (1.0 - {}) * (1.0 - {}))", lhs, rhs);
            }
            return;
        }
        if (!op_normal.contains(v.op)) {
            return setError();
        }
        accept(v.lhs);
        auto lhs = std::move(returned);
        accept(v.rhs);
        auto rhs = std::move(returned);
        returned = std::format("({} {} {})", lhs, v.op, rhs);
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        if ((v.op == "!" || v.op == "not") && topLevel) {
            accept(v.rhs);
            returned = std::format("(1.0 - {})", std::move(returned));
        } else if (v.op == "-" && !topLevel) {
            accept(v.rhs);
            returned = std::format("(0.0 - {})", std::move(returned));
        } else {
            return setError();
        }
    }
    // extinguish normal branch from ruleset branch
    VISIT_FUNCTION(BranchExprAST) { setError(); }
    VISIT_FUNCTION(ComplexLiteralExprAST) { setError(); }
    VISIT_FUNCTION(LoopAST) { setError(); }
    VISIT_FUNCTION(BlockExprAST) {
        if (v.exprs.size() == 1) {
            accept(v.exprs[0]);
        } else {
            setError();
        }
    }
    VISIT_FUNCTION(ControlFlowAST) { setError(); }
    VISIT_FUNCTION(TypeDefAST) { setError(); }
    VISIT_FUNCTION(VarDefAST) { setError(); }
    VISIT_FUNCTION(FunctionDefAST) { setError(); }
    VISIT_FUNCTION(SymbolDefAST) { setError(); }

  private:
    std::string newParaName();
    std::string newIndexName();
    template <typename Ty>
    void accept(Ty& expr) {
        if (err) {
            return;
        }
        expr->accept(this);
    }
    void setError(std::string_view = "") { err = true; }

    // temp member used by visit func to return value
    bool topLevel;
    std::string returned;
    std::vector<std::tuple<std::string, std::string>> states;
    std::vector<ParaInfo> paras;
    bool err;
    PYCodeGen* src;
};

struct PYCodeGen : public ASTVisitor {
    PYCodeGen(ContextStack& context, ruleset::RuleSetMetaInfo& metaInfo) : metaInfo(metaInfo){};
    std::string gen(std::unique_ptr<ExprAST>& e, size_t id, bool topLevel = true) {
        clear();
        isTopLevel = topLevel;
        subRuleSetID = id;
        e->accept(this);
        return mergeAllDef();
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        if (std::find(metaInfo.inputVar.begin(), metaInfo.inputVar.end(), v.name) != metaInfo.inputVar.end()) {
            returned = std::format("self.input_vars[\"{}\"]", v.name);
        } else if (std::find(metaInfo.outputVar.begin(), metaInfo.outputVar.end(), v.name) !=
                   metaInfo.outputVar.end()) {
            returned = std::format("self.output_vars[\"{}\"]", v.name);
        } else if (std::find(metaInfo.cacheVar.begin(), metaInfo.cacheVar.end(), v.name) != metaInfo.cacheVar.end()) {
            returned = std::format("self.temp_cached_vars[\"{}\"]", v.name);
        } else {
            returned = v.name;
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        if (auto p = dynamic_cast<LiteralExprAST*>(v.memberToken.get()); p && *(p->type) == StringType) {
            auto ans = acceptWithReturn(v.baseVar);
            returned = std::format("{}[\"{}\"]", ans, p->value);
        } else {
            returned = std::format("{}[{}]", acceptWithReturn(v.baseVar), acceptWithReturn(v.memberToken));
        }
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (*v.type == RealType || *v.type == IntType) {
            returned = v.value;
        } else if (*v.type == StringType) {
            returned = "\"\"\"" + v.value + "\"\"\"";
        } else if (v.type->isFunctionType()) {
            returned = v.value;
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        std::string paramList;
        for (auto&& p : v.params) {
            if (!paramList.empty()) {
                paramList.push_back(',');
            }
            paramList += acceptWithReturn(p);
        }
        // TODO: emit side effect?
        returned = std::format("{}({})", acceptWithReturn(v.functionIdent), paramList);
    }
    VISIT_FUNCTION(BinOpExprAST) {
        static const std::map<std::string, std::string> trans = {{"&&", "and"}, {"||", "or"}};
        auto op = v.op;
        if (auto it = trans.find(v.op); it != trans.end()) {
            op = it->second;
        }
        if (op == "=") {
            emitLine(std::format("{} = {}", acceptWithReturn(v.lhs), acceptWithReturn(v.rhs)));
        } else {
            returned = std::format("({} {} {})", acceptWithReturn(v.lhs), op, acceptWithReturn(v.rhs));
        }
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        static const std::map<std::string, std::string> trans = {{"!", "not"}};
        auto op = v.op;
        if (auto it = trans.find(v.op); it != trans.end()) {
            op = it->second;
        }
        returned = std::format("({} {})", op, acceptWithReturn(v.rhs));
    }
    // extinguish normal branch from ruleset branch
    VISIT_FUNCTION(BranchExprAST) {
        if (!isTopLevel) {
            if (*(v.type) != NoInstanceType) {
                auto id = getUnusedID("i");
                emitLine(std::format("{} = None", id));
                emitLine(std::format("if {}:", acceptWithReturn(v.condition)), 1);
                emitLine(std::format("{} = {}", id, acceptWithReturn(v.trueExpr)));
                emitEmptyLine(-1);
                emitLine("else:", 1);
                emitLine(std::format("{} = {}", id, acceptWithReturn(v.falseExpr)), -1);
                returned = std::move(id);
                return;
            }
            emitLine(std::format("if {}:", acceptWithReturn(v.condition)), 1);
            auto t = acceptWithReturn(v.trueExpr);
            emitLine((t.size() == 0) ? "pass" : t, -1);
            emitLine("else:", 1);
            auto f = acceptWithReturn(v.falseExpr);
            emitLine((f.size() == 0) ? "pass" : f, -1);
            return;
        }
        isTopLevel = false;

        codeClear();
        auto atomicRuleID = atomicRules.size();
        emitEmptyLine(2);
        emitLine(std::format("def action{}(self: SubRuleSet{}):", atomicRuleID, subRuleSetID), 1);
        acceptIgnoreReturn(v.trueExpr);
        atomicRules.push_back(std::move(code));

        codeClear();
        emitEmptyLine(2);
        emitLine(std::format("def condition{}(self: SubRuleSet{}):", atomicRuleID, subRuleSetID), 1);
        PYTransformCodeGen cg;
        auto ans = cg.gen(v.condition, *this);
        if (ans.has_value()) {
            auto&& [_code, _states, _paras] = ans.value();
            emitLine(std::format("return {}", _code));
            states.insert_range(states.end(), std::move(_states));
            paras.insert_range(paras.end(), std::move(_paras));
        } else {
            // TODO: toooo specific, need gernalize
            auto p1 = dynamic_cast<BlockExprAST*>(v.condition.get());
            if (auto p = dynamic_cast<BinOpExprAST*>(p1 ? p1->exprs[0].get() : nullptr);
                p && (p->op == "and" || p->op == "&&" || p->op == "or" || p->op == "||")) {
                PYTransformCodeGen cg;
                auto lhs = cg.gen(p->lhs, *this);
                auto rhs = cg.gen(p->rhs, *this);
                rulejit::ExprAST* tmp = p->lhs.get();
                if (lhs && !rhs) {
                    tmp = p->rhs.get();
                    swap(lhs, rhs);
                }
                if (rhs && !lhs) {
                    states.insert_range(states.end(), std::move(rhs.value().states));
                    paras.insert_range(paras.end(), std::move(rhs.value().paras));
                    auto s = getUnusedID("index");
                    if (p->op == "and" || p->op == "&&") {
                        emitLine(std::format("return self.state[:,self.{}] * ({})", s, rhs.value().code));
                    } else if (p->op == "or" || p->op == "||") {
                        emitLine(std::format("return 1.0 - ((1.0 - self.state[:,self.{}]) * (1.0 - ({})))", s, rhs.value().code));
                    }
                    states.emplace_back(std::move(s), std::format("1.0 if {} else 0.0", acceptWithReturn(tmp)));
                } else {
                    auto s = getUnusedID("index");
                    emitLine(std::format("return self.state[:,self.{}]", s));
                    states.emplace_back(std::move(s), std::format("1.0 if {} else 0.0", acceptWithReturn(v.condition)));
                }
            } else {
                auto s = getUnusedID("index");
                emitLine(std::format("return self.state[:,self.{}]", s));
                states.emplace_back(std::move(s), std::format("1.0 if {} else 0.0", acceptWithReturn(v.condition)));
            }
        }
        conditions.push_back(std::move(code));

        isTopLevel = true;
        codeClear();
        acceptIgnoreReturn(v.falseExpr);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        auto id = getUnusedID("i");
        if (v.type->isArrayType()) {
            emitLine(std::format("{} = []", id));
            for (auto& [_, value] : v.members) {
                emitLine(std::format("{}.append({})", id, acceptWithReturn(value)));
            }
        } else if (v.type->isBaseType()) {
            emitLine(std::format("{} = dict()", id));
            for (auto& [ident, value] : v.members) {
                emitLine(std::format("{}[\"{}\"] = {}", id, dynamic_cast<LiteralExprAST*>(ident.get())->value,
                                     acceptWithReturn(value)));
            }
        } else {
            // err
        }
        returned = id;
    }
    VISIT_FUNCTION(LoopAST) {
        emitLine(std::format("while {}:", acceptWithReturn(v.condition)), 1);
        acceptIgnoreReturn(v.body);
        emitEmptyLine(-1);
    }
    VISIT_FUNCTION(BlockExprAST) {
        if (v.exprs.empty()) {
            emitLine("pass");
            returned.clear();
            return;
        }
        for (auto&& expr : v.exprs | std::views::take(v.exprs.size() - 1)) {
            // TODO: emit? side-effect ignored
            acceptIgnoreReturn(expr);
        }
        returned = acceptWithReturn(v.exprs[v.exprs.size() - 1]);
    }
    VISIT_FUNCTION(ControlFlowAST) { return setError("ControlFlowAST not supported"); }
    VISIT_FUNCTION(TypeDefAST) { return setError("TypeDefAST not supported"); }
    VISIT_FUNCTION(VarDefAST) { emitLine(std::format("{} = {}", v.name, acceptWithReturn(v.definedValue))); }
    VISIT_FUNCTION(FunctionDefAST) { return setError("FunctionDefAST not supported"); }
    VISIT_FUNCTION(SymbolDefAST) { return setError("SymbolDefAST not supported"); }

  private:
    friend struct PYTransformCodeGen;
    template <typename Ty>
    std::string acceptWithReturn(Ty& expr) {
        returned.clear();
        expr->accept(this);
        return std::move(returned);
    }
    template <typename Ty>
    void acceptIgnoreReturn(Ty& expr) {
        returned.clear();
        expr->accept(this);
        if (!returned.empty()) {
            emitLine(std::move(returned));
            returned.clear();
        }
    }
    std::string mergeAllDef() {
        std::string actionRegister;
        for (size_t i = 0; i < atomicRules.size(); ++i) {
            actionRegister += atomicRules[i];
            actionRegister += std::format("\n        self.actions.append(action{0})\n", i);
        }
        actionRegister +=
            std::format("\n        def action{0}(self):\n            pass\n\n        self.actions.append(action{0})\n",
                        conditions.size());
        std::string conditionRegister;
        for (size_t i = 0; i < conditions.size(); ++i) {
            conditionRegister += conditions[i];
            conditionRegister += std::format("\n        self.conditions.append(condition{0})\n", i);
        }
        std::string parasRegister;
        for (auto [id, val] : paras) {
            parasRegister +=
                std::format("        self.{0} = torch.tensor([{1:.2f}], device=device, requires_grad=True)\n", id, val);
            parasRegister += std::format("        self.paras.append(self.{})\n", id);
        }
        std::string indexRegister;
        std::string stateCalculate;
        for (auto&& [id, v] : std::views::enumerate(states)) {
            auto&& [name, value] = v;
            indexRegister += std::format("        self.{} = {}\n", name, id);
            stateCalculate += std::format("        state.append({})\n", value);
        }
        // TODO:
        std::string ans = std::format(R"(
class SubRuleSet{}:
    def __init__(self, input_vars, output_vars, cached_vars):
        self.input_vars = input_vars
        self.output_vars = output_vars
        self.cached_vars = cached_vars
        self.temp_cached_vars = copy.deepcopy(cached_vars)
        self.paras = []
        self.conditions = []
        self.actions = []
        self.state = None
{}
{}
{}
{}

    def cal_state(self):
        self.temp_cached_vars = copy.deepcopy(self.cached_vars)
        state = []

{}
        self.state = torch.tensor([state], device=device, requires_grad=False)
        
    def forward(self, x=None):
        if x is not None:
            self.state = x
        prob = []
        p = torch.tensor([1.0], device=device, requires_grad=False)
        for con in self.conditions:
            p_this = p * con(self)
            prob.append(p_this)
            p = p - p_this
        prob.append(p)
        prob = torch.cat(prob)
        return prob
        )",
                                      subRuleSetID, std::move(actionRegister), std::move(conditionRegister),
                                      std::move(parasRegister), std::move(indexRegister), std::move(stateCalculate));
        return ans;
    }
    void emitLine(std::string_view line, int identChanged = 0) {
        code += line;
        emitEmptyLine(identChanged);
    }
    void emitEmptyLine(int identChanged = 0) {
        using namespace std::literals;
        ident += identChanged;
        code += '\n';
        code += tools::mystr::repeat("    "sv, ident);
    }
    void codeClear() {
        code.clear();
        ident = 0;
    }
    void clear() {
        ident = 0;
        nextUnusedID.clear();
        isTopLevel = true;

        paras.clear();
        conditions.clear();
        atomicRules.clear();
        states.clear();

        returned.clear();
        code.clear();
    }
    std::string getUnusedID(const std::string& domain) { return std::format("{}_{}", domain, nextUnusedID[domain]++); }
    SET_ERROR_MEMBER("PY Code Generation", void)

    ruleset::RuleSetMetaInfo& metaInfo;

    // generation config:
    size_t subRuleSetID;

    size_t ident;
    std::map<std::string, size_t, std::less<>> nextUnusedID;
    bool isTopLevel;
    std::vector<PYTransformCodeGen::ParaInfo> paras;
    std::vector<std::string> conditions;
    std::vector<std::string> atomicRules;
    std::vector<std::tuple<std::string, std::string>> states;

    // temp member used by visit func to return value
    std::string returned;
    // temp member used by visit func to emit code
    std::string code;
};

inline void PYTransformCodeGen::visit(IdentifierExprAST& v) {
    if (topLevel)
        return setError();
    if (std::find(src->metaInfo.inputVar.begin(), src->metaInfo.inputVar.end(), v.name) !=
        src->metaInfo.inputVar.end()) {
        returned = std::format("self.input_vars[\"{}\"]", v.name);
    } else if (std::find(src->metaInfo.outputVar.begin(), src->metaInfo.outputVar.end(), v.name) !=
               src->metaInfo.outputVar.end()) {
        returned = std::format("self.output_vars[\"{}\"]", v.name);
    } else if (std::find(src->metaInfo.cacheVar.begin(), src->metaInfo.cacheVar.end(), v.name) !=
               src->metaInfo.cacheVar.end()) {
        returned = std::format("self.temp_cached_vars[\"{}\"]", v.name);
    } else {
        return setError();
    }
}

inline std::string PYTransformCodeGen::newParaName() { return src->getUnusedID("p"); }
inline std::string PYTransformCodeGen::newIndexName() { return src->getUnusedID("index"); }

} // namespace rulejit::pybe