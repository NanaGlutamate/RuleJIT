#pragma once

#include <map>
#include <memory>
#include <string>
#include <format>
#include <source_location>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "tools/myassert.hpp"
#include "ast/context.hpp"

#define __RULEJIT_SEMANTIC_PUSH pushStack(v.get());
#define __RULEJIT_SEMANTIC_POP popStack();

namespace rulejit {

// 1. type inference
// 2. name unnamed complex type(in vardef, typedef, literal and funcdef)
// 3. scope process
// 4. capture analysis
// 5. redefined symbol name(var, type and func name cannot be same)
struct Semantic : public ASTVisitor {
    Semantic() = default;
    std::vector<AST*> callStack;
    // ContextStack cannot be destructed before last process call of this
    void loadContext(ContextStack& context){
        c = &context;
    }
    std::unique_ptr<AST> friend operator|(std::unique_ptr<AST> ast, Semantic &t) {
        return t.process(std::move(ast));
    }
    std::unique_ptr<AST> process(std::unique_ptr<AST> ast) {
        callStack.clear();
        type = nullptr;
        ast->accept(this);
        return std::move(ast);
    }
    VISIT_FUNCTION(IdentifierExprAST){
        processType(v.type);
        if(!v.type){
            auto [find, _, type] = c->seekVarDef(v.name);
            if(!find){
                return setError(std::format("Variable {} not found", v.name));
            }
            v.type = std::make_unique<TypeInfo>(type);
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST){
        processType(v.type);
        if(!v.type){
            v.baseVar->accept(this);
        }
    }
    VISIT_FUNCTION(LiteralExprAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(BinOpExprAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(BranchExprAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(LoopAST){
        processType(v.type);
        if(!v.type){
        }
    }
    VISIT_FUNCTION(BlockExprAST){
        // 1. process defs
        //    when meet func def, analysis captures
        //    and the define a unamed class to handle captured vars
        processType(v.type);
        if(!v.type){
        }
    }

    VISIT_FUNCTION(ControlFlowAST){
        processType(v.type);
        if(!v.type){
        }
    }

    VISIT_FUNCTION(TypeDefAST){
        if(v.typeDefType == TypeDefAST::TypeDefType::ALIAS){
            if(v.definedType->isComplexType()){
                return setError(std::format("type alias to unnamed type({}) is not allowed", v.definedType->toString()));
            }
        }
    }
    VISIT_FUNCTION(VarDefAST){
    }
    VISIT_FUNCTION(FunctionDefAST){
    }

  private:
    [[noreturn]] void setError(const std::string &info,
                                         const std::source_location location = std::source_location::current()) {

        throw std::logic_error(std::format("Type Check Error in {}::{}, line{}: {}", location.file_name(),
                                           location.function_name(), location.line(), info));
        // return nullptr;
    }
    void processType(std::unique_ptr<TypeInfo>& type){
        if(!type){return;}
        if(type->isSingleToken()){
            auto tmp = c->seekTypeAlias(type->idents[0]);
            while(std::get<0>(tmp)){
                type = std::make_unique<TypeInfo>(std::vector<std::string>{std::get<2>(tmp)});
                tmp = c->seekTypeAlias(type->idents[0]);
            }
        }
        if(type->isComplexType()){
            return setError(std::format("unnamed type {} is not allowed", type->toString()));
        }
    }
    // void pushStack(AST* v){callStack.push_back(v);}
    // void popStack(){callStack.pop_back();}
    std::unique_ptr<TypeInfo> type;
    ContextStack* c;
    std::set<std::string> captured;
};

} // namespace rulejit