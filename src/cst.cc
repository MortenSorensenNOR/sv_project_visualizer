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

static void ParseModuleDeclarationCST(const std::string& file, const json& module_decl) {
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

static void ParseModuleDeclarationFromJsonNode(const std::string& file, const json& node) {
    if (!node.is_object()) return;

    const std::string tag = tag_of(node);
    if (tag == "kModuleDeclaration") {
        ParseModuleDeclarationCST(file, node);
    }

    if (auto child_array = get_child_array(node)) {
        for (const auto& child : *child_array) {
            if (!child.is_null()) {
                ParseModuleDeclarationFromJsonNode(file, child);
            }
        }
    }
}

static void ParseModuleDeclarationsFromJSON(const std::string& file, const json& root_json) {
    if (!root_json.is_object()) return;

    auto it = root_json.find("tree");
    if (it != root_json.end() && it->is_object()) {
        ParseModuleDeclarationFromJsonNode(file, *it);
    } else {
        std::cout << "Hei\n";
        ParseModuleDeclarationsFromJSON(file, root_json);
    }
}

static void ParseModuleInstantiation(SV::Module* module, const json& module_inst_json) {
    if (!module_inst_json.is_object()) return;

    // Get module name
    auto instance_type = find_first(module_inst_json, "kInstantiationType");
    if (instance_type == nullptr) return;
    auto module_name = find_first(*instance_type, "SymbolIdentifier")->find("text")->get<std::string>();
    // TODO: Get parameter list wiht "kActualParameterList"
    
    // Get instantiation name
    auto instance_veriable_list = find_first(module_inst_json, "kGateInstanceRegisterVariableList");
    if (instance_veriable_list == nullptr) return;
    auto instantiation_name = find_first(*instance_veriable_list, "SymbolIdentifier")->find("text")->get<std::string>();
    // TODO: Parse port list

    auto instantiated_module_node = SymTable::symbol_table_lookup(global_module_symbol_table, module_name);
    if (instantiated_module_node == nullptr) return;
    module->dependencies.push_back(instantiated_module_node);
    instantiated_module_node->references.push_back(module);
}

static void ParseModuleInstantiationsFromModule(SV::Module* module) {
    if (!module->module_cst->is_object()) return;

    std::vector<const json*> module_instantiations;
    collect_all(*module->module_cst, "kInstantiationBase", module_instantiations);

    for (const auto& module_inst : module_instantiations) {
        ParseModuleInstantiation(module, *module_inst);
    }
}

// OLD CODE THAT DOES PORT LIST PARSING:
// static void ParseModuleInstantiationCSTNode(SVModuleNode* root, const json& module_inst) {
//     if (!module_inst.is_object()) return;
//
//     auto children    = get_child_array(module_inst);
//     std::string module_name = "";
//     std::string inst_name   = "";
//     std::vector<std::string> ports;
//
//     struct PortOrder {
//         SVPort port;
//         int    pos_end;
//        
//         PortOrder(SVPort& port, int pos_end) {
//             this->port = port;
//             this->pos_end = pos_end;
//         }
//
//         static bool sort(const PortOrder& a, const PortOrder& b) {
//             return a.pos_end < b.pos_end;
//         }
//     };
//     std::vector<PortOrder> unordered_ports; // Ports can be both named and unnamed, so for now, just sort based on when they appear in instantation, might be usefull later
//
//     // Get the name of the module
//     for (const auto& child : *children) {
//         // TODO: Figure out if the assumption that the first SymbolIdentifier is always the name
//         // we are after is true. I think it has to be wrt. SV syntax, but not quite certain.
//
//         if (tag_of(child) == "kInstantiationType") {
//             const auto* module_json_node = find_first(child, "SymbolIdentifier");
//             if (module_json_node == nullptr) {
//                 throw std::runtime_error("Could not find the module type");
//                 return;
//             }
//
//             auto module_name_it = module_json_node->find("text");
//             module_name = (module_name_it != module_json_node->end() && module_name_it->is_string()) ? 
//                            module_name_it->get<std::string>() : "";
//
//             // TODO: Find all parameter declarations
//         }
//         else if (tag_of(child) == "kGateInstanceRegisterVariableList") {
//             const auto* inst_json_node = find_first(child, "SymbolIdentifier");
//             if (inst_json_node == nullptr) {
//                 throw std::runtime_error("Could not find the instantiation name and name and registers");
//                 return;
//             }
//
//             auto inst_name_it = inst_json_node->find("text");
//             inst_name = (inst_name_it != inst_json_node->end() && inst_name_it->is_string()) ?
//                          inst_name_it->get<std::string>() : "";
//
//             // TODO: Find all register/signal invocations
//             const auto* port_list_json_node = find_first(child, "kPortActualList");
//             if (port_list_json_node == nullptr) {
//                 throw std::runtime_error("Could not find port list");
//                 return;
//             }
//
//             // Get ports that are positional
//             std::vector<const json*> positional_port_json_nodes;
//             collect_all(*port_list_json_node, "kActualPositionalPort", positional_port_json_nodes);
//
//             for (const auto& port : positional_port_json_nodes) {
//                 const auto* port_json_node = find_first(*port, "SymbolIdentifier");
//                 if (port_json_node == nullptr) {
//                     throw std::runtime_error("Could not find node for port delcaration");
//                     return;
//                 }
//
//                 // TODO: Figure out a way to find the port name from the module type we are instantiating
//                 int    port_end  = port_json_node->find("end")->get<int>();
//                 SVPort port_node = {.port_name = "", .signal_name = port_json_node->find("text")->get<std::string>(), .type = PORT_TYPE_POSITIONAL};
//                 unordered_ports.emplace_back(port_node, port_end);
//             }
//
//             // Get ports that are named
//             std::vector<const json*> named_port_json_nodes;
//             collect_all(*port_list_json_node, "kActualNamedPort", named_port_json_nodes);
//
//             for (const auto& port : named_port_json_nodes) {
//                 // Find the port
//                 const auto* port_json_node = find_first(*port, "SymbolIdentifier");
//                 if (port_json_node == nullptr) {
//                     throw std::runtime_error("Could not find node for port delcaration");
//                     return;
//                 }
//                 std::string port_name = port_json_node->find("text")->get<std::string>();
//
//                 // Find the signal
//                 const auto* signal_json_node = find_first(*nth_child(*port, 2), "SymbolIdentifier");
//                 if (signal_json_node == nullptr) {
//                     throw std::runtime_error("Could not find the signal for the named port");
//                     return;
//                 }
//                 std::string signal_name = signal_json_node->find("text")->get<std::string>();
//
//                 int    port_end  = port_json_node->find("end")->get<int>();
//                 SVPort port_node = {.port_name = port_name, .signal_name = signal_name, .type = PORT_TYPE_NAMED};
//                 unordered_ports.emplace_back(port_node, port_end);
//             }
//         }
//     }
//
//     // Sort ports so that positional ports still work
//     std::sort(unordered_ports.begin(), unordered_ports.end(), PortOrder::sort);
//
//     SVModuleNode* node = new SVModuleNode;
//     node->instance_name = inst_name;
//     node->module        = new SVModule;
//     node->module->module_name = module_name;
//    
//     // Set ports
//     node->module->IO_ports.reserve(unordered_ports.size());
//     for (int i = 0; i < unordered_ports.size(); i++) {
//         // std::cout << "Port name: " << port.port.port_name << ", Signal name: " << port.port.signal_name << "\n";
//         node->module->IO_ports.push_back(unordered_ports[i].port);
//         node->module->IO_ports.back().port_idx = i;
//     }
//
//     PrintModuleNode(node);
//    
//     // TODO: Insert into SVModuleNode* root tree
//     delete node->module;
//     delete node;
// }

[[nodiscard]] SV::Module* ParseCST(const json& cst_json) {
    // Check that we have a valid json
    if (!cst_json.is_object()) return nullptr;

    // Parse all module declarations
    global_module_symbol_table = new SymTable::ModuleSymbolTable;
    for (const auto& [filename, obj] : cst_json.items()) {
        ParseModuleDeclarationsFromJSON(filename, obj);
    }

    for (auto module : global_module_symbol_table->modules) {
        ParseModuleInstantiationsFromModule(module);
    }

    std::cout << "symbol table size: " << global_module_symbol_table->modules.size() << "\n";

    for (auto module : global_module_symbol_table->modules) {
        std::cout << module->source_file << ": " << module->name << "\n";
        std::cout << "Referenced by:\n";
        for (auto reference: module->references) {
            std::cout << "    - " << reference->name << "\n";
        }
        std::cout << "Depends on: \n";
        for (auto dependency: module->dependencies) {
            std::cout << "    - " << dependency->name << "\n";
        }
    }

    return nullptr;
}

}
