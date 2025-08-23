#include "common.h"

std::string ResolveUserPath(const char* arg) {
  namespace fs = std::filesystem;
  fs::path p(arg);
  if (p.is_absolute()) return p.string();

  const char* bwd = std::getenv("BUILD_WORKING_DIRECTORY");
  fs::path base = (bwd && *bwd) ? fs::path(bwd) : fs::current_path();
  // weakly_canonical tolerates non-existent intermediates better than canonical
  return fs::weakly_canonical(base / p).string();
}

std::string ReadAll(FILE* f) {
    std::ostringstream out;
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) out.write(buf, n);
    return out.str();
}
