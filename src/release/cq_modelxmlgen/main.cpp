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
#include <fstream>

#include "tools/mygetopt.hpp"
#include "xmlgen.hpp"

int main(int argc, const char **argv) {
    using namespace tools::myopt;
    CommandLineOpt opt;
    opt.head = "Usage: cq_modelxmlgen <input file name> [options] [flags]\n";

    opt.registerFlag({"-h", "--help", "-?"}, "Show this help message.");
    opt.registerArg({"-o", "--output"}, "Set output file path(\"./model.xml\" by default).");

    int cnt = opt.build(argc, argv);

    if (cnt < 0) {
        return 1;
    }

    bool help = opt.getFlag(false, "-h");
    if (help || cnt == 0) {
        std::cout << opt.getHelp() << std::endl;
    }

    std::string output = opt.getArg("./model.xml", "-o");
    if (std::filesystem::exists(output)) {
        // if already exists, ask user if continue
        if (!opt.askIfContinue("file " + output + " already exists, continue?")) {
            return 0;
        }
        std::string path = output.find_last_of('/') == std::string::npos ? output.substr(0, output.find_last_of('\\'))
                                                                         : output.substr(0, output.find_last_of('/'));
        std::filesystem::create_directory(path);
    }
    std::ofstream ofs(output);
}