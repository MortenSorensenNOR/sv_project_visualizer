#include "cst.h"
#include <stdexcept>

namespace cst {

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

json ParseFile(char* file_path, bazel::tools::cpp::runfiles::Runfiles* rf) {
    // Resolve the path for the file to parse
    const std::string sv_file = ResolveUserPath(file_path);

    // Locate the embedded Verible CLI in runfiles
    const std::string tool = rf->Rlocation(
            "verible~/verible/verilog/tools/syntax/verible-verilog-syntax");

    std::string cmd  = tool + " --export_json --printtree " + sv_file;
    FILE*       pipe = popen(cmd.c_str(), "r");
    if (!pipe) { 
        std::string err_msg = "failed to exec: " + cmd;
        throw std::runtime_error(err_msg);
    }

    std::string verible_cst_json_str = ReadAll(pipe);
    int         rc                   = pclose (pipe);
    if (rc != 0) {
        std::string err_msg = "verible returned " + std::to_string(rc);
        throw std::runtime_error(err_msg);
    }
    std::cout << "Got: " << verible_cst_json_str.size() << " bytes of CST JSON\n";

    return json::parse(verible_cst_json_str);
}

}
