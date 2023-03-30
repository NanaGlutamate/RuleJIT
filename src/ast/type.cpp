/**
 * @file type.cpp
 * @author djw
 * @brief AST/Type
 * @date 2023-03-28
 * 
 * @details Includes some definitions of member functions of TypeInfo
 * must include defines/typedef.hpp due to TypeInfo::getReturnedType
 * may need to return NoInstanceType, and may introduce loop dependency
 * if put function defines in type.hpp, so put them in type.cpp
 * 
 * deprecated
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Move to hpp.</td></tr>
 * </table>
 */

// #include "type.hpp"
// #include "defines/typedef.hpp"

// namespace rulejit {

// const TypeInfo &TypeInfo::getReturnedType() const {
//     if (isNoReturnFunctionType()) {
//         return NoInstanceType;
//     } else {
//         my_assert(isReturnedFunctionType(), "only function type has return type");
//         return subTypes.back();
//     }
// }

// } // namespace rulejit