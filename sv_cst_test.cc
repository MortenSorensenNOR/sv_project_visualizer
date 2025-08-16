#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "tools/cpp/runfiles/runfiles.h" // from @bazel_tools


static std::string ResolveUserPath(const char* arg) {
  namespace fs = std::filesystem;
  fs::path p(arg);
  if (p.is_absolute()) return p.string();

  const char* bwd = std::getenv("BUILD_WORKING_DIRECTORY");
  fs::path base = (bwd && *bwd) ? fs::path(bwd) : fs::current_path();
  // weakly_canonical tolerates non-existent intermediates better than canonical
  return fs::weakly_canonical(base / p).string();
}

static std::string ReadAll(FILE* f) {
    std::ostringstream out;
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) out.write(buf, n);
    return out.str();
}

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "usage: sv_cst_test <file.sv>\n"; return 2; }

    const std::string sv_file = ResolveUserPath(argv[1]);

    std::string error;
    auto* rf = bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error);
    if (!rf) { std::cerr << "runfiles error: " << error << "\n"; return 1; }

    // Locate the embedded Verible CLI in runfiles
    const std::string tool = rf->Rlocation(
        "verible~/verible/verilog/tools/syntax/verible-verilog-syntax");

    // Fallback: if not found in runfiles, try PATH
    std::string cmd  = tool + " --export_json --printtree " + sv_file;
    FILE*       pipe = popen(cmd.c_str(), "r");
    if (!pipe) { std::cerr << "failed to exec: " << cmd << "\n"; return 1; }

    std::string verible_cst_json_str = ReadAll(pipe);
    int         rc                   = pclose (pipe);
    if (rc != 0) { std::cerr << "verible returned " << rc << "\n"; return 1; }
    std::cout << "Got: " << verible_cst_json_str.size() << " bytes of CST JSON\n";

    // Parse json
    json cst_json = json::parse(verible_cst_json_str);
    std::cout << std::setw(4) << cst_json << "\n\n";
}
