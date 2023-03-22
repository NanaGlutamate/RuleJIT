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