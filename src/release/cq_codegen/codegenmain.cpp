#include <filesystem>
#include <iostream>
#include <string>

#include "backend/cppbe/cppengine.h"

int main(int argc, char **argv) {
    if (argc == 1 || argc == 2 && argv[1] == std::string_view("--help")) {
        std::cout << "Usage:  cq_codegen --help | cq_codegen [options]\n"
                     "Options:\n"
                     "        <input file>        Set the input file.\n"
                     "\n"
                     "        -outpath <path>     Set the output path.\n"
                     "        -namespace <name>   Set the namespace name.\n"
                     "        -prefix <prefix>    Set the prefix for generated file.\n"
                     "\n";
        return 0;
    }
    rulejit::cppgen::CppEngine codegen;
    std::string in;
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string_view("-outpath")) {
            i++;
            if (i == argc) {
                std::cout << "No output path specified." << std::endl;
                return 1;
            }
            codegen.setOutputPath(argv[i]);
        } else if (argv[i] == std::string_view("-namespace")) {
            i++;
            if (i == argc) {
                std::cout << "No namespace specified." << std::endl;
                return 1;
            }
            codegen.setNamespaceName(argv[i]);
        } else if (argv[i] == std::string_view("-prefix")) {
            i++;
            if (i == argc) {
                std::cout << "No prefix specified." << std::endl;
                return 1;
            }
            codegen.setPrefix(argv[i]);
        } else if (in.empty()) {
            in = argv[i];
        } else {
            std::cout << "Unknown argument: " << argv[i] << std::endl;
            return 1;
        }
    }
    if (in.empty()) {
        std::cout << "No input file specified." << std::endl;
        return 1;
    }
    // check if codegen.outputPath exists, if not, create it.
    if (!std::filesystem::exists(codegen.outputPath)) {
        std::filesystem::create_directories(codegen.outputPath);
    } else {
        std::cout << "directory " << codegen.outputPath << " already exists, continue?[Y/n]" << std::endl;
        std::string input;
        std::cin >> input;
        if (input == "n" || input == "N") {
            return 0;
        } else if (input != "" && input != "y" && input != "Y") {
            std::cout << "invalid input, abort." << std::endl;
            return 1;
        }
    }
    // std::cout << "write to " << codegen.outputPath << std::endl;
    try {
        codegen.buildFromFile(in);
    } catch (std::exception &e) {
        std::cout << "Generate not complete, error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}