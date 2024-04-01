/**
 * @file rulesetxmlparser.h
 * @author djw
 * @brief 
 * @date 2023-05-08
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-05-08</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "rulesetparser.h"

namespace rulejit::xmlparser {

/// @brief XML parser
struct XMLParser{

    /**
     * @brief parse source XML to RuleSetStructure
     * 
     * @param srcXML source XML
     * @return ruleset::RuleSetStructure 
     */
    static ruleset::RuleSetStructure parseXML(std::vector<char> srcXML);

};
    
}