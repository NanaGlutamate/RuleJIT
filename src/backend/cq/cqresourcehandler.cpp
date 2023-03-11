#include "cqresourcehandler.h"

namespace {

rulejit::cq::ResourceHandler* handlerCurrent = nullptr;

}

extern "C" {

void cq_change_provider(void* tar){
    handlerCurrent = reinterpret_cast<rulejit::cq::ResourceHandler*>(tar);
}

size_t cq_load(const char *s) {
    std::string tmp = s;
    return handlerCurrent->readIn(tmp);
}

}

