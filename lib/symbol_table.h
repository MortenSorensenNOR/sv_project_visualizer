#pragma once

#include <unordered_map>

#include "common.h"
#include "sv.h"

namespace SymTable {

/**
 * @brief Symbol table of all module delcarations
 * @var modules Vector of module pointers inside the symbol table
 * @var hashmap Unordered map from string (name of module) to int (index into "modules" array)
 */
struct ModuleSymbolTable {
    std::vector<SV::Module*> modules;
    std::unordered_map<std::string, size_t> hashmap;
};

enum InsertResult {
    INSERT_OK = 0,
    INSERT_COLLISION = 1
};

/**
 * @brief Insert a module into the symbol table
 * @param table  Pointer to symbol table of modules to be inserted into
 * @param module Pointer to module struct to be inserted
 * @return InsertResult type. INSERT_OK if the insert was successfull, or INSERT_COLLISION if a collision occured
 */
SymTable::InsertResult symbol_table_insert(SymTable::ModuleSymbolTable* table, SV::Module* module);

/**
 * @brief Retrieve a module fro the symbol table by name reference
 * @param table Pointer to symbol table of modules to do the lookup in
 * @param name  Name of the module to be found
 * @return SV::Module*, pointer to the module in the symbol table that has the same name as "name". If not found, return nullptr.
 */
SV::Module* symbol_table_lookup(const SymTable::ModuleSymbolTable* table, const std::string& name);

/**
 * @brief Destroy the symbol table and the modules inside it
 */
void symbol_table_destroy(SymTable::ModuleSymbolTable* table);

}
