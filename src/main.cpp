#include "common.h"
#include "graphics.h"
#include "cst.h"
#include <symbol_table.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "usage: sv_cst_test <file.sv> [file_2.sv ...]\n"; return 2; }

    // Initialize the window
    graphics::initWindow(1920, 1080, true); 

    // Create runfile
    std::string error;
    auto* rf = bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error);
    if (!rf) { std::cerr << "runfiles error: " << error << "\n"; return 1; }

    // Parse and colorize a .sv file
    sv::ColorizedDoc g_doc = sv::ColorizeFileViaBazelRunfiles(argv[1], rf, {});
    std::cout << "g_doc size: " << g_doc.size() << "\n";;

    for (auto& line : g_doc) {
        for (auto& word : line) {
            std::cout << word;
        }
        std::cout << "\n";
    }
    
    // Parse json
    size_t num_files = argc - 1;
    json cst_json = cst::ParseFiles(num_files, &argv[1], rf); // Assumes that all parameters passed are files to parse

    // Parse CST json file
    SV::Module* root = cst::ParseCST(cst_json);
    
    // Main loop
    while (graphics::updateWindow(root, g_doc)) {} // TODO: make it so that there is a start and end thing so i can put stuff here perhaps

    return 0;
}
