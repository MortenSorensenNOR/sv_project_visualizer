#include "common.h"
#include "graphics.h"

int main(int argc, char** argv) {
    std::string error;
    auto* rf = bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error);
    if (!rf) { std::cerr << "runfiles error: " << error << "\n"; return 1; }

    // Parse and colorize a .sv file
    sv::ColorizedDoc g_doc = sv::ColorizeFileViaBazelRunfiles("test/example.sv", rf, {});
    std::cout << "g_doc size: " << g_doc.size() << "\n";;

    for (auto& line : g_doc) {
        for (auto& word : line) {
            std::cout << word;
        }
        std::cout << "\n";
    }

    graphics::initWindow(1920, 1080, true); 
    
    // Main loop
    while (graphics::updateWindow(g_doc)) {} // TODO: make it so that there is a start and end thing so i can put stuff here perhaps

    return 0;
}
