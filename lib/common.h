#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "vec.h"

#include "tools/cpp/runfiles/runfiles.h" // from @bazel_tools

// Path
std::string ResolveUserPath(const char* arg);

// Json stuff
inline bool is_object(const json& n) { return n.is_object(); }
inline bool is_array(const json& n)  { return n.is_array(); }

inline const json* get_child_array(const json& node) {
    auto it = node.find("children");
    return (it != node.end() && it->is_array()) ? &*it : nullptr;
}

inline const json* nth_child(const json& node, size_t i) {
    if (auto a = get_child_array(node)) {
        if (i < a->size() && !(*a)[i].is_null()) return &(*a)[i];
    }
    return nullptr;
}

inline std::string tag_of(const json& n) {
    auto it = n.find("tag");
    return (it != n.end() && it->is_string()) ? it->get<std::string>() : "";
}

inline std::optional<std::string> symbol_text(const json& n) {
    if (tag_of(n) == "SymbolIdentifier") {
        auto it = n.find("text");
        if (it != n.end() && it->is_string()) return it->get<std::string>();
    }
    return std::nullopt;
}

inline const json* find_first(const json& node, std::string_view wanted_tag) {
    if (!is_object(node)) return nullptr;
    if (tag_of(node) == wanted_tag) return &node;
    if (auto a = get_child_array(node)) {
        for (const auto& c : *a) if (!c.is_null()) {
            if (auto* r = find_first(c, wanted_tag)) return r;
        }
    }
    return nullptr;
}

// collect all descendants with a given tag
inline void collect_all(const json& node, std::string_view wanted_tag, std::vector<const json*>& out) {
    if (!is_object(node)) return;
    if (tag_of(node) == wanted_tag) out.push_back(&node);
    if (auto a = get_child_array(node)) {
        for (const auto& c : *a) if (!c.is_null()) collect_all(c, wanted_tag, out);
    }
}

inline const json* find_first_recursive(const json& node, std::string_view wanted_tag) {
    const auto* current_layer = find_first(node, wanted_tag);
    if (current_layer != nullptr) return current_layer;
    for (const auto& e : *get_child_array(node)) {
        const auto* child_layer = find_first_recursive(e, wanted_tag);
        if (child_layer != nullptr) return child_layer;
    }
    return nullptr;
}
