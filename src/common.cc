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
