/**
 * @file xmlgen.hpp
 * @author djw
 * @brief
 * @date 2023-04-27
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <array>
#include <format>
#include <fstream>
#include <string>

#include "frontend/ruleset/rulesetparser.h"

namespace rulejit::xmlgen {

// name, displayname, dllname, category, parameters
inline constexpr auto filetemplate = R"(<?xml version="1.0" encoding="UTF-8"?>
<ModelInfo id="" name="{}" displayName="{}" dllName="{}" hasGeoInfo="false" category="{}" multiples="">
	<Parameters>
		<Parameter name="ID" type="uint64" displayName="ID" usage="init,output" value="" unit=""/>
		<Parameter name="InstanceName" type="string" displayName="instance_name" usage="init,output" value="" unit=""/>
		<Parameter name="ForceSideID" type="uint16" displayName="force_side" usage="init,output" value="" unit=""/>
		<Parameter name="ModelID" type="string" displayName="model_ID" usage="init,output" value="" unit=""/>
		<Parameter name="KeyMessages" type="string[]" displayName="key_messages" usage="output" value="" unit=""/>
		<Parameter name="State" type="uint16" displayName="state" usage="output" value="" unit=""/>{}
    </Parameters>
</ModelInfo>
)";

// name, type, usage
inline constexpr auto paramtemplate = R"(
		<Parameter name="{0}" type="{1}" displayName="{0}" usage="{2}" value="" unit=""/>)";

struct ModelXMLGenerator {
    std::string dllName, name, displayName, category;
    void gen(const std::string &dst, const std::string &src) {
        using namespace std;
        ofstream out(dst);
        ifstream in(src);
        ContextStack context;
        ruleset::RuleSetMetaInfo data;
        ruleset::RuleSetParser::readSource(
            std::string{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()}, context, data);
        string params;
        for (auto &[usage, varTable] : std::array<std::pair<std::string, std::vector<std::string> *>, 2>{
                 std::pair{"input", &data.inputVar}, std::pair{"output", &data.outputVar}}) {
            for (auto &name : *varTable) {
                params += std::format(paramtemplate, name, data.varType[name], usage);
            }
        }
        out << std::format(filetemplate, name, displayName, dllName, category, params);
    }
};

} // namespace rulejit::xmlgen