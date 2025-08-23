#pragma once
#include "common.h"
#include "sv.h"

namespace cst {

enum PortType {
    PORT_TYPE_NAMED,
    PORT_TYPE_POSITIONAL,
};

typedef struct {
    std::string port_name;
    std::string signal_name; // TODO: Include ranged stuff perhaps?
                             // TODO: Figure out if this should be here? Only valid for instantiation perhaps?
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

    std::string source_file;
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
struct SVModuleNode {
    std::string instance_name;
    SVModule*   module;

    // TODO: Have some reference to the actual files/modules in the project or symbol table
    
    SVModuleNode* parent;
    std::vector<SVModuleNode*> children;

    SVModuleNode() {
        instance_name = "";
        module = nullptr;

        parent   = nullptr;
        children.clear();
    }
};

/**
 * @brief Take in a single system verilog file, run it through the verible parser, and 
 * return the json CST output.
 */
json ParseFiles(size_t num_files, char** file_paths, bazel::tools::cpp::runfiles::Runfiles* rf);

/**
 * @brief Takes in a json CST and parses out the module structure of the cst.
 */
// SVModuleNode* ParseCST(const json& cst_json);
SV::Module* ParseCST(const json& cst_json);


/**
 * @brief Pretty print the node structure
 */
void PrintModuleNode(const SVModuleNode* node, int indent = 0);

} // end namespace cst
