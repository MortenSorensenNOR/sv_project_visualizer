#pragma once
#include "common.h"

namespace cst {

json ParseFile(char* file_path, bazel::tools::cpp::runfiles::Runfiles* rf);

} // end namespace cst
