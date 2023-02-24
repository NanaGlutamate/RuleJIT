#pragma once

#include <string>

namespace rulejit {

enum class VarType{
    REAL,
    INT,
    STRING,
};

struct Ast{
    virtual ~Ast() = default;
};

struct Identifier : public Ast{
    std::string name;
};

struct Immediate : public Ast{

};

}