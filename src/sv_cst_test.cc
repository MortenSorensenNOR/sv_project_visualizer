#include "common.h"
#include "cst.h"

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "usage: sv_cst_test <file.sv> [file_2.sv ...]\n"; return 2; }

    // Get runfiles
    std::string error;
    auto* rf = bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error);
    if (!rf) { std::cerr << "runfiles error: " << error << "\n"; return 1; }

    // Parse json
    size_t num_files = argc - 1;
    json cst_json = cst::ParseFiles(num_files, &argv[1], rf); // Assumes that all parameters passed are files to parse

    // Parse CST json file
    SV::Module* cst_tree = cst::ParseCST(cst_json);
    // delete cst_tree;

    return 0;
}
