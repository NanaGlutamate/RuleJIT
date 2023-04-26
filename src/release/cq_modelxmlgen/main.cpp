/**
 * @file main.cpp
 * @author djw
 * @brief
 * @date 2023-04-26
 *
 * @details tools to automatically generate model xml for cq
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-26</td><td>Initial version.</td></tr>
 * </table>
 */
#include <filesystem>
#include <iostream>

#include "tools/mygetopt.hpp"
#include "xmlgen.hpp"

int main(int argc, const char **argv) {
    using namespace tools::myopt;
    CommandLineOpt opt;
    rulejit::xmlgen::ModelXMLGenerator gen;

    opt.head = "Usage: cq_modelxmlgen <input file name> [options] [flags]\n";

    opt.registerFlag({"-h", "--help", "-?"}, "Show this help message");
    opt.registerArg({"-o", "--output"}, "Set output file path(\"./model.xml\" by default)");
    // dllName, name, displayName, category
    opt.registerArg({"-dl", "--dllName"}, "Set dllName(\"cq_interpreter\" by default)");
    opt.registerArg({"-n", "--name"}, "Set name(\"UnnamedBehaviourModel\" by default)");
    opt.registerArg({"-ds", "--displayName"}, "Set displayName(same as dllName by default)");
    opt.registerArg({"-c", "--category"}, "Set category(\"ATOMIC_ENTITY\" by default)");

    int cnt = opt.build(argc, argv);
    // int cnt = opt.innerBuild({"-o", "test.xml", "D:\\Desktop\\FinalProj\\Code\\RuleJIT\\doc\\test_xml\\car_rule.xml"});

    if (cnt < 0) {
        return 1;
    }

    bool help = opt.getFlag(false, "-h");
    if (help || cnt == 0) {
        std::cout << opt.getHelp() << std::endl;
        return 0;
    }

    std::string output = opt.getArg("./model.xml", "-o");
    gen.dllName = opt.getArg("cq_interpreter", "-dl");
    gen.name = opt.getArg("UnnamedBehaviourModel", "-n");
    gen.displayName = opt.getArg(gen.dllName, "-ds");
    gen.category = opt.getArg("ATOMIC_ENTITY", "-c");

    std::string in;
    for (auto s : opt.unspecifiedValue) {
        if(!in.empty()){
            std::cout << "too many input files specified." << std::endl;
            return 1;
        }
        in = s;
    }
    if (in.empty()) {
        std::cout << "No input file specified." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(in)){
        std::cout << "input file " << in << " not exists." << std::endl;
        return 1;
    }

    if (std::filesystem::exists(output)) {
        // if already exists, ask user if continue
        if (!opt.askIfContinue("file " + output + " already exists, continue?", false)) {
            return 0;
        }
    } else {
        if (auto index = output.find_last_of('/'); index != std::string::npos) {
            std::filesystem::create_directory(output.substr(0, index));
        } else if (index = output.find_last_of('\\'); index != std::string::npos) {
            std::filesystem::create_directory(output.substr(0, index));
        }

    }
    gen.gen(output, in);
}