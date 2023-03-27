/**
 * @file type.cpp
 * @author djw
 * @brief AST/Type
 * @date 2023-03-28
 * 
 * @details Includes some definitions of member functions of TypeInfo
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#include "type.hpp"
#include "defines/typedef.hpp"

namespace rulejit {

TypeInfo::TypeInfo(const std::string &s) {
    static ExpressionLexer lexer;
    *this = s | lexer | TypeParser();
}

const TypeInfo &TypeInfo::getReturnedType() const {
    if (isNoReturnFunctionType()) {
        return NoInstanceType;
    } else {
        my_assert(isReturnedFunctionType(), "only function type has return type");
        return subTypes.back();
    }
}

} // namespace rulejit