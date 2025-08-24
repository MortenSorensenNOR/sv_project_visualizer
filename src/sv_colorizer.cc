// sv_colorizer.cpp (relevant bits)

#include <cctype>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include "common.h"
#include "sv_colorizer.h"

// ---------- token classification ----------
namespace {

static Color COL_COMMENT = Color(0x828A9AFFu); 

static Color COL_TEXT    = Color(0xE1E3E4FFu); 
static Color COL_STRING  = Color(0xFFD670FFu); 
static Color COL_NUMBER  = Color(0xFFD670FFu); 
static Color COL_KEYWORD = Color(0xFF70A6FFu); 
static Color COL_TYPE    = Color(0x70D6FFFFu); 
static Color COL_PP      = Color(0xFF9770FFu);
static Color COL_SYMBOL  = Color(0xFF9770FFu);
static Color COL_IDENT   = Color(0xD2EFFAFFu);

// Types you already had
static const std::unordered_set<std::string> kTypeWords = {
    "logic","bit","byte","shortint","int","longint","integer","time",
    "shortreal","real","realtime","string",
    "wire","uwire","tri","tri0","tri1","supply0","supply1","wand","wor",
    "signed","unsigned","struct","union","enum","typedef"
};

// Common SV keywords (add more as you need)
static const std::unordered_set<std::string> kKeywords = {
    "module","endmodule","input","output","inout","parameter","localparam",
    "assign","begin","end","if","else","for","while","case","endcase",
    "always","always_ff","always_comb","always_latch","generate","endgenerate",
    "package","endpackage","import","export","typedef","function","endfunction",
    "task","endtask","interface","endinterface","class","endclass"
};

// Common punctuation / operator tokens Verible emits as tags
static const std::unordered_set<std::string> kSymbolTags = {
    "(", ")", "[", "]", "{", "}", ",", ";", ":", ".", "#",
    "+", "-", "*", "/", "%", "&", "|", "^", "~", "!", "?", "<", ">", "=",
    "&&", "||", "==", "!=", "<=", ">=", "<<", ">>", "+=", "-=", "*=", "/=", "%=",
    "&=", "|=", "^=", "<<=", ">>=", "->", "::", ":=", "@"
};

// Return true if this token is punctuation/operator. Prefer tag, fall back to lex slice.
inline bool IsSymbolToken(const std::string& tag, const std::string& lex) {
    if (kSymbolTags.count(tag)) return true;
    if (kSymbolTags.count(lex)) return true;
    // Single-character punctuation heuristic
    if (tag.size() == 1 && std::ispunct(static_cast<unsigned char>(tag[0]))) return true;
    if (lex.size() == 1 && std::ispunct(static_cast<unsigned char>(lex[0]))) return true;
    return false;
}

inline bool IsWhitespaceToken(const std::string& tag) {
    // Verible emits TK_SPACE (spaces/tabs) and TK_NEWLINE
    return tag == "TK_SPACE" || tag == "TK_NEWLINE";
}

inline bool LooksComment(const std::string& lex) {
    return lex.rfind("//", 0) == 0 || lex.rfind("/*", 0) == 0;
}

inline bool LooksStringToken(const std::string& tag) {
    return tag == "TK_StringLiteral";
}

inline bool LooksNumberToken(const std::string& tag, const std::string& lex) {
    if (tag.find("Number") != std::string::npos) return true; // TK_DecNumber, TK_HexNumber, …
    // Fallback if tag is generic: digits or SystemVerilog number like 'h, 'd, 'b
    return !lex.empty() && (std::isdigit(static_cast<unsigned char>(lex[0])) || lex[0] == '\'');
}

inline bool LooksPP(const std::string& tag, const std::string& lex) {
    if (!lex.empty() && lex[0] == '`') return true;              // `define, `include, macro use
    return tag.rfind("PP_", 0) == 0;                              // any preproc token
}

inline bool IsKeywordTag(const std::string& tag) {
    // Verible spells many keywords as the tag text itself (e.g., "module", "assign")
    return kKeywords.count(tag) > 0;
}

inline bool IsTypeTag(const std::string& tag) {
    // Type words also arrive as their word as the tag (e.g., "logic", "string")
    if (kTypeWords.count(tag)) return true;
    // Some parsed forms carry "DataType" in tag
    return tag.find("DataType") != std::string::npos;
}

inline bool IsSymbolIdentifier(const std::string& tag) {
    if (tag == "SymbolIdentifier") {
        return true;
    }
    return false;
}

inline Color ColorFor(const std::string& tag, const std::string& lex) {
    if (IsWhitespaceToken(tag))          return COL_TEXT;
    if (LooksComment(lex))               return COL_COMMENT;
    if (LooksStringToken(tag))           return COL_STRING;
    if (LooksNumberToken(tag, lex))      return COL_NUMBER;
    if (LooksPP(tag, lex))               return COL_PP;
    if (IsTypeTag(tag))                  return COL_TYPE;
    if (IsKeywordTag(tag))               return COL_KEYWORD;
    if (IsSymbolToken(tag, lex))         return COL_SYMBOL;   // <— new line
    if (IsSymbolIdentifier(tag))         return COL_IDENT;
    return COL_TEXT; // identifiers, etc.
}

static void AppendWhitespaceExpanded(const std::string& s, int tab_spaces,
                                     sv::LineSpans& line) {
    for (char ch : s) {
        if (ch == '\t') line.push_back({std::string(tab_spaces, ' '), COL_TEXT});
        else if (ch != '\r') line.push_back({std::string(1, ch), COL_TEXT});
    }
}

static std::string ReadFile(const std::string& p) { 
    std::ifstream f(p, std::ios::binary); 
    if (!f) throw std::runtime_error("Failed to open: " + p); 
    std::ostringstream ss; ss << f.rdbuf(); 
    return ss.str();
}

static sv::ColorizedDoc BuildDocFromVeribleJSON(const json& j,
                                                const std::string& filepath,
                                                const std::string& source,
                                                int tab_spaces)
{
    sv::ColorizedDoc doc;
    if (!j.contains(filepath)) return doc;

    const json& fobj = j.at(filepath);
    const bool has_raw = fobj.contains("rawtokens");
    const json& toks = has_raw ? fobj["rawtokens"]
                               : (fobj.contains("tokens") ? fobj["tokens"] : json::array());

    sv::LineSpans current;

    for (const auto& t : toks) {
        const std::string tag = t.at("tag").get<std::string>();
        size_t a = t.at("start").get<size_t>();
        size_t b = t.at("end").get<size_t>();
        if (b > source.size()) b = source.size();
        std::string lex = source.substr(a, b - a);

        if (IsWhitespaceToken(tag)) {
            // Verible usually gives text for TK_SPACE/TK_NEWLINE too, but compute from source anyway
            if (lex == "\n") {
                doc.push_back(std::move(current));
                current = sv::LineSpans{};
            } else {
                AppendWhitespaceExpanded(lex, tab_spaces, current);
            }
            continue;
        }

        // Split on embedded newlines for safety
        size_t p = 0;
        while (true) {
            size_t nl = lex.find('\n', p);
            const std::string piece = (nl == std::string::npos) ? lex.substr(p)
                                                                : lex.substr(p, nl - p);
            if (!piece.empty()) current.push_back({piece, ColorFor(tag, piece)});
            if (nl == std::string::npos) break;
            doc.push_back(std::move(current));
            current = sv::LineSpans{};
            p = nl + 1;
        }
    }

    doc.push_back(std::move(current));
    return doc;
}

} // namespace

namespace sv {

ColorizedDoc ColorizeFileViaBazelRunfiles(const char* file_path, bazel::tools::cpp::runfiles::Runfiles* rf, const ColorizerOpts& opt) {
    const std::string sv_file = ResolveUserPath(file_path);
    const std::string tool = rf->Rlocation("verible~/verible/verilog/tools/syntax/verible-verilog-syntax");

    std::ostringstream cmd;
    cmd << tool << " --export_json --printrawtokens " << sv_file;
    for (const auto& a : opt.extra_args) cmd << ' ' << a;

    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) throw std::runtime_error(std::string("failed to exec: ") + cmd.str());

    std::string verible_json_str = ReadAll(pipe);
    int rc = pclose(pipe);
    if (rc != 0) throw std::runtime_error("verible returned " + std::to_string(rc));

    json j = json::parse(verible_json_str);
    const std::string src = ReadFile(sv_file);
    return BuildDocFromVeribleJSON(j, sv_file, src, opt.tab_spaces);
}

} // namespace sv

// Your TextSpan printer stays the same:
std::ostream& operator<<(std::ostream& os, const sv::TextSpan& span) {
    return os << span.color << span.text << "\033[0m";
}
