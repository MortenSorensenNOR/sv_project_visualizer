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
                                         
struct Range {
    int start;
    int end;

    Range() {
        start = 0;
        end   = 0;
    }

    Range(int start, int end) {
        this->start = start;
        this->end   = end;
    }

    vec2 get() const {
        return vec2(start, end);
    }
};

struct Color {
    uint8_t r8, g8, b8, a8; // canonical storage

    // Constructors
    Color(uint8_t r=255, uint8_t g=255, uint8_t b=255, uint8_t a=255)
        : r8(r), g8(g), b8(b), a8(a) {}

    Color(float r, float g, float b, float a=1.0f) {
        r8 = static_cast<uint8_t>(r * 255.0f);
        g8 = static_cast<uint8_t>(g * 255.0f);
        b8 = static_cast<uint8_t>(b * 255.0f);
        a8 = static_cast<uint8_t>(a * 255.0f);
    }

    Color(uint32_t packed) {
        r8 = (packed >> 24) & 0xFF;
        g8 = (packed >> 16) & 0xFF;
        b8 = (packed >> 8)  & 0xFF;
        a8 = (packed)       & 0xFF;
    }

    // Accessors
    float r() const { return r8 / 255.0f; }
    float g() const { return g8 / 255.0f; }
    float b() const { return b8 / 255.0f; }
    float a() const { return a8 / 255.0f; }

    uint8_t& r() { return r8; }
    uint8_t& g() { return g8; }
    uint8_t& b() { return b8; }
    uint8_t& a() { return a8; }

    uint32_t rgba() const {
        return (uint32_t(r8) << 24) |
               (uint32_t(g8) << 16) |
               (uint32_t(b8) << 8)  |
               (uint32_t(a8));
    }
};

// Stream operator for Color (ANSI escape code)
std::ostream& operator<<(std::ostream& os, const Color& c);

// Path
std::string ResolveUserPath(const char* arg);

// File stuff
std::string ReadAll(FILE* f);

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
