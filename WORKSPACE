load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Include cc rules
http_archive(
    name = "rules_cc",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/refs/tags/0.0.10.zip"],
    strip_prefix = "rules_cc-0.0.10",
)

# Provide an empty user config so Skia doesn't complain
local_repository(
    name = "skia_user_config",
    path = ".",
)

local_repository(
    name = "buildifier_prebuilt",
    path = ".",
)

http_archive(
    name = "skia",
    urls = ["https://github.com/google/skia/archive/refs/heads/main.zip"],
    strip_prefix = "skia-main",  # fetch full repo
)
