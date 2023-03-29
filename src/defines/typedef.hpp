/**
 * @file typedef.hpp
 * @author djw
 * @brief Defines/Type defines
 * @date 2023-03-28
 * 
 * @details Includes some type defines.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include "ast/type.hpp"

namespace rulejit {

/// @brief Used for type for no returned expression
inline const TypeInfo NoInstanceType{std::vector<std::string>{std::string(typeident::NoInstanceTypeIdent)}};
/// @brief Type for string
inline const TypeInfo StringType{std::vector<std::string>{std::string(typeident::StringTypeIdent)}};
/// @brief type for integer, same as RealType for now
inline const TypeInfo IntType{std::vector<std::string>{std::string(typeident::IntTypeIdent)}};
/// @brief type for real number
inline const TypeInfo RealType{std::vector<std::string>{std::string(typeident::RealTypeIdent)}};
/// @brief auto type for type inference in var-def expression, auto generate only and cannot use by user
inline const TypeInfo AutoType{std::vector<std::string>{std::string(typeident::AutoTypeIdent)}};
/// @brief type for unary function and operator
inline const TypeInfo BuildInUnaryType = make_type("func(f64):f64");

/// @brief build in types
inline const std::set<TypeInfo> BuildInType{
    NoInstanceType,
    StringType,
    IntType,
    RealType,
};

inline std::unique_ptr<TypeInfo> getAuto() { return std::make_unique<TypeInfo>(AutoType); }

} // namespace rulejit