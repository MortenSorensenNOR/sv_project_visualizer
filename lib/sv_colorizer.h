// sv_colorizer.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "common.h"

namespace sv {

// One colored run of text.
struct TextSpan { std::string text; Color color; };
using LineSpans    = std::vector<TextSpan>;
using ColorizedDoc = std::vector<LineSpans>;

struct ColorizerOpts {
    int  tab_spaces = 4;                 // expand \t to spaces
    bool include_whitespace = true;      // expect rawtokens (<<\n>> etc.)
    std::vector<std::string> extra_args; // e.g. {"-I", "inc", "--define=FOO=1"}
};

// ---- Bazel/runfiles entry points ----

// Multiple files → map: filepath -> ColorizedDoc
ColorizedDoc ColorizeFileViaBazelRunfiles(const char* file_path, bazel::tools::cpp::runfiles::Runfiles* rf, const ColorizerOpts& opt);

} // namespace sv

// Stream operator for TextSpan
std::ostream& operator<<(std::ostream& os, const sv::TextSpan& span);
