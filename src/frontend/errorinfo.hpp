/**
 * @file errorinfo.hpp
 * @author djw
 * @brief
 * @date 2023-04-24
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-24</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"

namespace rulejit {

// struct DebugInfo {};

std::string genErrorInfo(const std::vector<ExprAST *> &callStack, const std::map<ExprAST *, std::string_view> &ast2place,
                         const std::vector<const char *> &linePointer, const char * now, const std::string &errorType) {
    std::string info;
    ExprAST* nearestAST = nullptr;
    std::string_view ASTText;
    for (auto it = callStack.rbegin(); it != callStack.rend(); it++) {
        if (auto p = ast2place.find(*it); p != ast2place.end()) {
            std::tie(nearestAST, ASTText) = *p;
            break;
        }
    }
    if (!nearestAST) {
        return "No position record found.";
    }
    return info;
}

} // namespace rulejit
