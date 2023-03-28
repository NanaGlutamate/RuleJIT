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

/// @brief NoInstanceType used for type for no returned expression
inline const TypeInfo NoInstanceType{std::vector<std::string>{std::string(typeident::NoInstanceTypeIdent)}};
inline const TypeInfo StringType{std::vector<std::string>{std::string(typeident::StringTypeIdent)}};
inline const TypeInfo IntType{std::vector<std::string>{std::string(typeident::IntTypeIdent)}};
inline const TypeInfo RealType{std::vector<std::string>{std::string(typeident::RealTypeIdent)}};
inline const TypeInfo AutoType{std::vector<std::string>{std::string(typeident::AutoTypeIdent)}};
inline const TypeInfo BuildInUnaryType{"func(f64):f64"};

inline const std::set<TypeInfo> BuildInType{
    NoInstanceType,
    StringType,
    IntType,
    RealType,
};

inline std::unique_ptr<TypeInfo> getAuto() { return std::make_unique<TypeInfo>(AutoType); }

} // namespace rulejit