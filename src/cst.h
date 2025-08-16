#pragma once
#include "common.h"

namespace cst {


/**
 * @brief TODO
 * @params
 *     module_name      -    name of the module
 */
typedef struct {
    char*  module_name;
    char** IO_ports;
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
    char* instance_name;
    SVModule* module;

    // TODO: If instantiation, have port mapping of signals to ports
    // TODO: Have some reference to the actual files/modules in the project or symbol table

    size_t      n_children;
    ModuleNode* children;
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

} // end namespace cst
