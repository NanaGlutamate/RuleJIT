/**
 * @file irholder.hpp
 * @author djw
 * @brief IR/IR holder
 * @date 2023-03-28
 *
 * @details Includes IRHolder which holds generated function
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <memory>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

namespace rulejit::ir {

using namespace llvm;

struct IRHolder {
    IRHolder() {
        context = std::make_unique<LLVMContext>();
        builder = std::make_unique<IRBuilder<>>(*context);
        module = std::make_unique<Module>("rulejit", *context);
    }

    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<IRBuilder<>> builder;
    std::unique_ptr<Module> module;
    std::map<std::string, Value *> NamedValues;
};

} // namespace rulejit::ir