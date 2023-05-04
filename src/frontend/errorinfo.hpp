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

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "ast/ast.hpp"
#include "tools/stringprocess.hpp"

namespace rulejit {

struct ErrorLocation {
    std::string_view preline;
    std::string_view line;
    size_t start, length;
    std::string genIdentifier() const { return tools::mystr::repeat(" ", start) + tools::mystr::repeat("^", length); }
    std::string concatenateIdentifier(size_t maxCharPerLine = 80, size_t ident = 4) const {
        std::string line1, line2, ret;
        size_t totalCnt = 0;
        for (auto &c : line) {
            if (totalCnt % maxCharPerLine == 0) {
                ret += std::move(line1) + "\n" + std::move(line2) + "\n";
                line1.clear();
                line2.clear();
            }
            line1 += isspace(c) ? ' ' : c;
            line2 += (totalCnt >= start && totalCnt < start + length) ? '^' : ' ';
            totalCnt++;
        }
        if (!line1.empty()) {
            ret += std::move(line1) + "\n" + std::move(line2) + "\n";
        }
        return ret;
    }
};

/**
 * @brief generate error information from state of lexer, parser and semantic.
 * (call stack, ast2place, linePointer and now)
 *
 * @param callStack call stack of semantic
 * @param ast2place location map of parser
 * @param linePointer location map of lexer
 * @param now current pointer of lexer
 * @return ErrorLocation
 */
ErrorLocation genErrorInfo(const std::vector<ExprAST *> &callStack,
                           const std::map<ExprAST *, std::string_view> &ast2place,
                           const std::vector<const char *> &linePointer, const char *now, const char *next) {
    ExprAST *nearestAST = nullptr;
    std::string_view ASTText;
    for (auto it = callStack.rbegin(); it != callStack.rend(); it++) {
        if (auto p = ast2place.find(*it); p != ast2place.end()) {
            std::tie(nearestAST, ASTText) = *p;
            break;
        }
    }
    if (!nearestAST && !linePointer.empty()) {
        // not start semantic check yet, means error caused by parser/lexer
        // so to indicate the curent token
        auto begin = now;
        while (*now != '\n' && *now != '\0') {
            now++;
        }
        auto tmp = std::string_view{linePointer.back(), now};
        std::string_view pre;
        if (linePointer.size() > 1) {
            pre = std::string_view{*(linePointer.end() - 2), linePointer.back()};
        }
        return ErrorLocation{pre, tmp, static_cast<size_t>(begin - linePointer.back()),
                             static_cast<size_t>(next - linePointer.back())};
    } else if (linePointer.empty()) {
        // not start lexer yet, no info to show
        return ErrorLocation{"", "", 0, 0};
    }
    const char *begin = linePointer.back();
    const char *end = now;
    const char *pre = nullptr;
    for (auto it = linePointer.rbegin(); it != linePointer.rend(); it++) {
        if (*it > &ASTText[0]) {
            end = begin;
            begin = *it;
        } else {
            end = begin;
            begin = *it;
            if (it != linePointer.rend() - 1) {
                pre = *(it + 1);
            } else {
                pre = begin;
            }
            break;
        }
    }
    size_t start = &ASTText[0] - begin;
    size_t length = ASTText.size();
    return ErrorLocation{std::string_view{pre, begin}, std::string_view{begin, end}, start, length};
}

} // namespace rulejit
