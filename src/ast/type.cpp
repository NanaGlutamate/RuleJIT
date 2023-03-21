#include "type.hpp"
#include "defines/typedef.hpp"

namespace rulejit {

const TypeInfo &TypeInfo::getReturnedType() const {
    if (isNoReturnFunctionType()) {
        return NoInstanceType;
    } else {
        my_assert(isReturnedFunctionType(), "only function type has return type");
        return subTypes.back();
    }
}

} // namespace rulejit