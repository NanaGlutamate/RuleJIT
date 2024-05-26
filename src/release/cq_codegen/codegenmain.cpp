/**
 * @file codegenmain.cpp
 * @author djw
 * @brief CQ/CPPBE/Command line tools
 * @date 2023-03-27
 * 
 * @details Provides a command line tool to generate cpp code from ruleset XML
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#include <filesystem>
#include <iostream>
#include <string>

#include "backend/cppbe/cppengine.h"
#include "tools/mygetopt.hpp"

int main(int argc, const char **argv) {
    using namespace tools::myopt;
    CommandLineOpt opt;
    opt.head = "Usage: cq_codegen <input file name> [options] [flags]\n";

    opt.registerFlag({"-h", "--help", "-?"}, "Show this help message.");

    opt.registerArg({"-o", "--outpath"}, "Set the output path(\"./src/\" by default)");
    opt.registerArg({"-n", "--namespace"}, "Set the namespace name(\"ruleset\" by default)");
    opt.registerArg({"-p", "--prefix"}, "Set the prefix for generated file(empty by default)");

    int cnt = opt.build(argc, argv);

    if(cnt < 0){
        return 1;
    }

    bool help = opt.getFlag(false, "-h");
    if (help || cnt == 0) {
        std::cout << opt.getHelp() << std::endl;
        return 0;
    }

    rulejit::cppgen::CppEngine codegen;
    std::string outpath = opt.getArg("./src/", "-o");
    codegen.setOutputPath(outpath);
    std::string ns = opt.getArg("ruleset", "-n");
    codegen.setNamespaceName(ns);
    std::string prefix = opt.getArg("", "-p");
    codegen.setPrefix(prefix);
    std::string in;
    for (auto s : opt.unspecifiedValue) {
        if(!in.empty()){
            std::cout << "too many input files specified." << std::endl;
            return 1;
        }
        in = s;
    }
    // process command line arguments
    if (in.empty()) {
        std::cout << "No input file specified." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(in)){
        std::cout << "input file " << in << " not exists." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(codegen.outputPath)) {
        // check if codegen.outputPath exists, if not, create it.
        std::filesystem::create_directories(codegen.outputPath);
    } 
    // donot ask to enable direct use in codegenUI
    // else {
    //     // if already exists, ask user if continue
    //     if (!opt.askIfContinue("directory " + codegen.outputPath + " already exists, continue?")) {
    //         return 0;
    //     }
    // }
    // start code generation, catch exceptions while throwed, and print exception message.
    try {
        codegen.buildFromFile(in);
    } catch (std::exception &e) {
        std::cout << "Generation not complete, error: \n" << e.what() << std::endl;
        return 1;
    }
    return 0;
}