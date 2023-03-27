/**
 * @file cqresourcehandler.cpp
 * @author djw
 * @brief CQ/Interpreter/CQResourceHandler
 * @date 2023-03-27
 * 
 * @details C interface for JIT-ed sub rule set,
 * designed in Service Provider Pattern(https://developer.aliyun.com/article/265421).
 * unused for now
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#include "cqresourcehandler.h"

namespace {

rulejit::cq::ResourceHandler* handlerCurrent = nullptr;

}

extern "C" {

void cq_change_provider(void* tar){
    handlerCurrent = reinterpret_cast<rulejit::cq::ResourceHandler*>(tar);
}

}

