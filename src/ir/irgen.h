#pragma once

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "ast/astvisitor.hpp"

namespace rulejit {

struct IRGenerator : public ASTVisitor {
    VISIT_FUNCTION(IdentifierExprAST);
    VISIT_FUNCTION(MemberAccessExprAST);
    VISIT_FUNCTION(LiteralExprAST);
    VISIT_FUNCTION(FunctionCallExprAST);
    // VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);
    // VIRTUAL_VISIT_FUNCTION(BranchExprAST);
    // VIRTUAL_VISIT_FUNCTION(LoopAST);
    // VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    // VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    // VIRTUAL_VISIT_FUNCTION(VarDefAST);
    // VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    // VIRTUAL_VISIT_FUNCTION(TopLevelAST);
};

}