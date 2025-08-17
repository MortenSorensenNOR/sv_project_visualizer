#include "cst.h"
#include <stdexcept>

namespace cst {

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

static void ParseModuleDeclarationCSTNode(ModuleNode* root, const json& module_decl) {
    std::cout << "found module declaration\n";
    // std::cout << "Module decl: \n";
    // std::cout << std::setw(4) << module_decl << "\n\n\n";
}

static void ParseModuleInstantiationCSTNode(ModuleNode* root, const json& module_inst) {
    if (!module_inst.is_object()) return;

    auto children    = get_child_array(module_inst);
    std::string module_name = "";
    std::string inst_name   = "";
    std::vector<std::string> ports;

    struct PortOrder {
        SVPort port;
        int    pos_end;
        
        PortOrder(SVPort& port, int pos_end) {
            this->port = port;
            this->pos_end = pos_end;
        }

        static bool sort(const PortOrder& a, const PortOrder& b) {
            return a.pos_end < b.pos_end;
        }
    };
    std::vector<PortOrder> unordered_ports; // Ports can be both named and unnamed, so for now, just sort based on when they appear in instantation, might be usefull later

    // Get the name of the module
    for (const auto& child : *children) {
        // TODO: Figure out if the assumption that the first SymbolIdentifier is always the name
        // we are after is true. I think it has to be wrt. SV syntax, but not quite certain.

        if (tag_of(child) == "kInstantiationType") {
            const auto* module_json_node = find_first_recursive(child, "SymbolIdentifier");
            if (module_json_node == nullptr) {
                throw std::runtime_error("Could not find the module type");
                return;
            }

            auto module_name_it = module_json_node->find("text");
            module_name = (module_name_it != module_json_node->end() && module_name_it->is_string()) ? 
                           module_name_it->get<std::string>() : "";

            // TODO: Find all parameter declarations
        }
        else if (tag_of(child) == "kGateInstanceRegisterVariableList") {
            const auto* inst_json_node = find_first_recursive(child, "SymbolIdentifier");
            if (inst_json_node == nullptr) {
                throw std::runtime_error("Could not find the instantiation name and name and registers");
                return;
            }

            auto inst_name_it = inst_json_node->find("text");
            inst_name = (inst_name_it != inst_json_node->end() && inst_name_it->is_string()) ?
                         inst_name_it->get<std::string>() : "";

            // TODO: Find all register/signal invocations
            const auto* port_list_json_node = find_first_recursive(child, "kPortActualList");
            if (port_list_json_node == nullptr) {
                throw std::runtime_error("Could not find port list");
                return;
            }

            // Get ports that are positional
            std::vector<const json*> positional_port_json_nodes;
            collect_all(*port_list_json_node, "kActualPositionalPort", positional_port_json_nodes);

            for (const auto& port : positional_port_json_nodes) {
                const auto* port_json_node = find_first_recursive(*port, "SymbolIdentifier");
                if (port_json_node == nullptr) {
                    throw std::runtime_error("Could not find node for port delcaration");
                    return;
                }

                // TODO: Figure out a way to find the port name from the module type we are instantiating
                int    port_end  = port_json_node->find("end")->get<int>();
                SVPort port_node = {.port_name = "", .signal_name = port_json_node->find("text")->get<std::string>()};
                unordered_ports.emplace_back(port_node, port_end);
            }

            // Get ports that are named
            std::vector<const json*> named_port_json_nodes;
            collect_all(*port_list_json_node, "kActualNamedPort", named_port_json_nodes);

            for (const auto& port : named_port_json_nodes) {
                // Find the port
                const auto* port_json_node = find_first_recursive(*port, "SymbolIdentifier");
                if (port_json_node == nullptr) {
                    throw std::runtime_error("Could not find node for port delcaration");
                    return;
                }
                std::string port_name = port_json_node->find("text")->get<std::string>();

                // Find the signal
                const auto* signal_json_node = find_first_recursive(*nth_child(*port, 2), "SymbolIdentifier");
                if (signal_json_node == nullptr) {
                    throw std::runtime_error("Could not find the signal for the named port");
                    return;
                }
                std::string signal_name = signal_json_node->find("text")->get<std::string>();

                int    port_end  = port_json_node->find("end")->get<int>();
                SVPort port_node = {.port_name = port_name, .signal_name = signal_name};
                unordered_ports.emplace_back(port_node, port_end);
            }
        }
    }

    // Sort ports so that positional ports still work
    std::sort(unordered_ports.begin(), unordered_ports.end(), PortOrder::sort);

    ModuleNode* node = new ModuleNode;
    node->instance_name = inst_name;
    node->module        = new SVModule;
    node->module->module_name = module_name;
    
    // Set ports
    node->module->IO_ports.reserve(unordered_ports.size());
    for (auto& port : unordered_ports) {
        // std::cout << "Port name: " << port.port.port_name << ", Signal name: " << port.port.signal_name << "\n";
        node->module->IO_ports.push_back(port.port);
    }

    PrintModuleNode(node);
    
    // TODO: Insert into ModuleNode* root tree
    delete node->module;
    delete node;
}

static void ParseModulesFromCSTNode(ModuleNode* root, const json& node) {
    if (!node.is_object()) return;

    const std::string t = tag_of(node);
    if (t == "kModuleDeclaration") {
        ParseModuleDeclarationCSTNode(root, node);
    }

    if (t == "kInstantiationBase") {
        ParseModuleInstantiationCSTNode(root, node);
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
    ModuleNode* root = new ModuleNode;
    for (const auto& [filename, obj] : cst_json.items()) {
        ParseModulesFromJSON(root, obj);
    }

    return root;
}

void PrintModuleNode(const ModuleNode* node, int indent) {
    if (!node) return;

    std::string pad(indent, ' ');

    std::cout << pad << "Instance: "
              << (node->instance_name.empty() ? "<top>" : node->instance_name)
              << "\n";

    if (node->module) {
        std::cout << pad << "  Module: " << node->module->module_name << "\n";

        if (!node->module->IO_ports.empty()) {
            std::cout << pad << "  Ports:\n";

            // Find the max port_name length
            size_t max_len = 0;
            for (const auto& port : node->module->IO_ports) {
                max_len = std::max(max_len, port.port_name.size());
            }

            // Print with alignment
            for (const auto& port : node->module->IO_ports) {
                std::cout << pad << "    - "
                          << std::left << std::setw(max_len) << port.port_name;

                if (!port.signal_name.empty()) {
                    std::cout << " ↔  " << port.signal_name;
                }
                std::cout << "\n";
            }
        }
    } else {
        std::cout << pad << "  Module: <null>\n";
    }

    // Print children
    for (size_t i = 0; i < node->n_children; i++) {
        PrintModuleNode(&node->children[i], indent + 4);
    }
}

}
