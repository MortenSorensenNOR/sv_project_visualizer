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

static void ParseModulesFromCSTNode(ModuleNode* root, const json& node) {
    if (!node.is_object()) return;

    const std::string t = tag_of(node);
    if (t == "kModuleDeclaration") {
        std::cout << "found module declaration\n";
    }

    if (t == "kInstantiationBase") {
        std::cout << "found module instantiation\n";
    }

    if (auto a = get_child_array(node)) {
        for (const auto& c : *a) {
            if (!c.is_null()) {
                // TODO: craete new modulenode node I think right?
                ParseModulesFromCSTNode(root, c);
            }
        }
    }
}

static void ParseModulesFromJSON(ModuleNode* root, const json& root_json) {
    if (!root_json.is_object()) return;
    auto it = root_json.find("tree");
    if (it != root_json.end() && it->is_object()) {
        // TODO: Create new node in the tree and make sure that it actually has children and or data
        ParseModulesFromCSTNode(root, *it);
    } else {
        // TODO: Create module here as well I guess
        ParseModulesFromJSON(root, root_json);
    }
}

[[nodiscard]] ModuleNode* ParseCST(const json& cst_json) {
    // Check that we have a valid json
    if (!cst_json.is_object()) return nullptr;

    // Parse 
    ModuleNode* root = (ModuleNode*)malloc(sizeof(ModuleNode));
    for (const auto& [filename, obj] : cst_json.items()) {
        ParseModulesFromJSON(root, obj);
    }

    return root;
}

}
