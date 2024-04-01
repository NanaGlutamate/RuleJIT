/**
 * @file showmsg.hpp
 * @author djw
 * @brief 
 * @date 2023-04-22
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-22</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "defines/marco.hpp"


namespace rulejit{

inline std::vector<std::string> debugMessages;

inline void debugMsg(const std::string& msg){
#ifndef __RULEJIT_NO_MSG
    std::cout << msg << std::endl;
    debugMessages.push_back(msg);
#endif // __RULEJIT_NO_MSG
}

}
