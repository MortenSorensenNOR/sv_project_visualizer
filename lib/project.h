#pragma once

#include <unordered_map>

#include "common.h"
#include "sv.h"

struct ModuleSymbolTable {
    std::vector<SV::Module*> modules;
    std::unordered_map<std::string, int> hashmap;
};
