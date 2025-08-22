#include "common.h"
#include "cst.h"

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "usage: sv_cst_test <file.sv>\n"; return 2; }

    // Get runfiles
    std::string error;
    auto* rf = bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error);
    if (!rf) { std::cerr << "runfiles error: " << error << "\n"; return 1; }

    // Parse json
    json cst_json = cst::ParseFile(argv[1], rf);

    // Parse CST json file
    cst::SVModuleNode* cst_tree = cst::ParseCST(cst_json);
    delete cst_tree;

    return 0;
}
