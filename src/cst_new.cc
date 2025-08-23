#include "common.h"
#include "symbol_table.h"
#include "cst.h"

namespace cst {

SymTable::ModuleSymbolTable* global_module_symbol_table;

json ParseFiles(size_t num_files, char** file_paths, bazel::tools::cpp::runfiles::Runfiles* rf) {
    // Parse multiple files
    std::vector<std::string> files_to_parse;

    for (int i = 0; i < num_files; i++) {
        // Resolve the path for the file to parse
        const std::string sv_file = ResolveUserPath(file_paths[i]);
        files_to_parse.push_back(sv_file);
    }

    std::string all_files = "";
    for (auto& file : files_to_parse) {
        all_files += file + " ";
    }

    // Locate the embedded Verible CLI in runfiles
    const std::string tool = rf->Rlocation("verible~/verible/verilog/tools/syntax/verible-verilog-syntax");

    std::string cmd  = tool + " --export_json --printtree " + all_files;
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

static void ParseModleDeclarationCST(const std::string& file, const json& module_decl) {
    if (!module_decl.is_object()) return;

    // Parse a module declaration
    SV::Module* module = new SV::Module;
    module->source_file = file;
    module->module_cst  = &module_decl;

    auto module_header = find_first(module_decl, "kModuleHeader");
    if (module_header == nullptr) {
        throw std::runtime_error("Could not find module header of module delcaration");
    }

    auto children = get_child_array(*module_header);
    for (const auto& child : *children) {
        auto child_tag = tag_of(child);

        if (child_tag == "SymbolIdentifier") {
            auto module_name_it = child.find("text");
            if (module_name_it != child.end()) {
                module->name = module_name_it->get<std::string>();
            } else {
                throw std::runtime_error("Could not find name of module");
            }
        } else if (child_tag == "kFormalParameterListDeclaration") {
            // Parameters
            // TODO: Implement
        } else if (child_tag == "kParentGroup") {
            // Ports
            std::vector<const json*> ports;
            auto port_list = find_first(child, "kPortDeclarationList");
            collect_all(*port_list, "kPortDeclaration", ports);

            for (const auto port : ports) {
                // TODO
            }
        }
    }
    
    // Insert module into symbol table
    SymTable::symbol_table_insert(global_module_symbol_table, module);
    
}

static void ParseModuleFromCSTNode(const std::string& file, const json& node) {
    if (!node.is_object()) return;

    const std::string tag = tag_of(node);
    if (tag == "kModuleDeclaration") {
        ParseModleDeclarationCST(file, node);
    }

    if (auto child_array = get_child_array(node)) {
        for (const auto& child : *child_array) {
            if (!child.is_null()) {
                ParseModuleFromCSTNode(file, child);
            }
        }
    }
}

static void ParseModulesFromJSON(const std::string& file, const json& root_json) {
    if (!root_json.is_object()) return;

    auto it = root_json.find("tree");
    if (it != root_json.end() && it->is_object()) {
        ParseModuleFromCSTNode(file, *it);
    } else {
        ParseModulesFromJSON(file, root_json);
    }
}

[[nodiscard]] SV::Module* ParseCST(const json& cst_json) {
    // Check that we have a valid json
    if (!cst_json.is_object()) return nullptr;

    // Parse all module declarations
    global_module_symbol_table = new SymTable::ModuleSymbolTable;
    for (const auto& [filename, obj] : cst_json.items()) {
        ParseModulesFromJSON(filename, obj);
    }

    std::cout << "symbol table size: " << global_module_symbol_table->modules.size() << "\n";

    for (auto module : global_module_symbol_table->modules) {
        std::cout << module->source_file << ": " << module->name << "\n";
    }

    return nullptr;
}

}
