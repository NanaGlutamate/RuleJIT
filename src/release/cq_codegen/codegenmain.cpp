#include <iostream>
#include <string>

#include "backend/cppbe/cppengine.h"

int main(int argc, char **argv) {
    if (argc == 1 || argc == 2 && argv[1] == std::string_view("--help")) {
        std::cout << R"(Usage: cq_codegen [options] <input file> [options]
Options:
    --help              Show this help message.

    -outpath <path>     Set the output path.
    -namespace <name>   Set the namespace name.
    -prefix <prefix>    Set the prefix for generated file.

)";
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
        } else if (in.empty() && i == argc - 1) {
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
    try {
        codegen.buildFromFile(in);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}