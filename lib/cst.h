#pragma once
#include "common.h"

namespace cst {

enum PortType {
    PORT_TYPE_NAMED,
    PORT_TYPE_POSITIONAL,
};

typedef struct {
    std::string port_name;
    std::string signal_name; // TODO: Include ranged stuff perhaps?
    int port_idx;
    PortType type;
} SVPort;

/**
 * @brief TODO
 * @params
 *     module_name      -    name of the module
 */
typedef struct {
    std::string         module_name;
    std::vector<SVPort> IO_ports;
    // TODO: Add a port type to track ports better
} SVModule;

/**
 * @brief Node graph built from the parsed CST from verible containing the structure of the project in 
 * terms of the modules used.
 * @params
 *     instance_name    -    name of instantiation ("" string if top level module)
 *     module           -    pointer to a struct containing information about the module
 *     n_children       -    number of children of this node
 *     children         -    array of child nodes
 */
typedef struct ModuleNode {
    std::string instance_name;
    SVModule*   module;

    // TODO: If instantiation, have port mapping of signals to ports
    // TODO: Have some reference to the actual files/modules in the project or symbol table
    
    ModuleNode*  parent;
    ModuleNode** children;
    size_t       n_children;

    ModuleNode() {
        instance_name = "";
        module = nullptr;

        parent   = nullptr;
        children = nullptr;
        n_children = 0;
    }
} ModuleNode;

/**
 * @brief Take in a single system verilog file, run it through the verible parser, and 
 * return the json CST output.
 */
json ParseFile(char* file_path, bazel::tools::cpp::runfiles::Runfiles* rf);

/**
 * @brief Takes in a json CST and parses out the module structure of the cst.
 */
ModuleNode* ParseCST(const json& cst_json);

/**
 * @brief Pretty print the node structure
 */
void PrintModuleNode(const ModuleNode* node, int indent = 0);

} // end namespace cst
