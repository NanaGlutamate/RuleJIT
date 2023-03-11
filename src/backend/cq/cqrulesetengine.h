#pragma once

#include "rapidxml-1.13/rapidxml.hpp"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"

namespace rulejit::cq {

struct RuleSetEngine{
    void loadFile();
    void init(){
        handler.Init();
        interpreter.setResourceHandler(&handler);
    };
    void tick();
private:
    ResourceHandler handler;
    ExpressionLexer lexer;
    ExpressionParser parser;
    CQInterpreter interpreter;
};

}