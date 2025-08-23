#pragma once

#include "common.h"

namespace SV {

enum DataType {
    /* VERILOG SUBSET */
    REG,
    WIRE,
    INTEGER,
    REAL,
    TIME,
    REALTIME,
    /* VERILOG SUBSET END */
    LOGIC,
    BIT,
    BYTE,
    SHORTINT,
    INT,
    LONGINT,
    SHORTREAL
};

enum PortType {
    INPUT,
    OUTPUT,
    INOUT
};

enum PortInstanceType {
    POSITIONAL,
    NAMED,
};

struct Port {
    std::string name;

    SV::PortType port_type;
    SV::DataType data_type;

    bool has_packed_dims   = false;
    bool has_unpacked_dims = false;
    Range packed_dim   = Range();
    Range inpacked_dim = Range();
};

struct InstancePort {
    SV::PortInstanceType port_type;
    SV::Port* module_port;
    std::string signal_name; // TODO: Figure out if I should make a signal struct to fix this? 
                             // Not important for now
};

struct Parameter {
    std::string name;
    std::string default_value; // TODO: Figure out if this should be string or literal
    SV::DataType data_type;
};

struct Module {
    std::string name;
    std::string source_file;

    std::vector<Port>      ports;
    std::vector<Parameter> parameters;

    std::vector<Module*> references;   // Modules that reference this module
    std::vector<Module*> dependencies; // Modules that this module depends on (instantiations)

    // TODO: How do I connect it to the JSON parsing?
    // After constructing symbol table, still need to parse module instantiations
};

}
