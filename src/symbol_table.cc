#include "common.h"
#include "symbol_table.h"


SymTable::InsertResult SymTable::symbol_table_insert(SymTable::ModuleSymbolTable* table, SV::Module* module) {
    if (table == nullptr || module == nullptr) {
        return SymTable::InsertResult::INSERT_COLLISION;
    }

    auto it = table->hashmap.find(module->name);
    if (it != table->hashmap.end()) {
        // Module of the same name already exists
        return SymTable::InsertResult::INSERT_COLLISION;
    }

    table->modules.push_back(module);
    table->hashmap.emplace(module->name, table->modules.size() - 1);
    return SymTable::InsertResult::INSERT_OK;
}

SV::Module* SymTable::symbol_table_lookup(const SymTable::ModuleSymbolTable* table, const std::string& name) {
    if (table == nullptr) return nullptr;

    auto it = table->hashmap.find(name);
    if (it == table->hashmap.end()) return nullptr;

    auto idx = it->second;
    if (idx >= table->modules.size()) return nullptr;
    return table->modules[idx];
}
