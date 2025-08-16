cc_library (
    name = "sv_core",
)

cc_binary(
    name = "sv_cst_test",
    srcs = [
        "src/sv_cst_test.cc",
        "src/cst.cc",
    ],
    hdrs = [
        "src/common.h",
        "src/cst.h",
    ],
    includes = ["src"],
    deps = [
        "@bazel_tools//tools/cpp/runfiles", # to find runfiles portably
        "@nlohmann_json//:json",
    ],
    data = ["@verible//verible/verilog/tools/syntax:verible-verilog-syntax"],  # bundle the tool
)

# compile_commands.json generation
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_compile_commands",
    # Only index your C++ targets (add more as you like).
    targets = {
        "//:sv_cst_test": "--cxxopt=-std=gnu++17",
        # "//:other_target": "",
    },
    # Skip headers from external repos (avoids the Abseil header action).
    exclude_headers = "external",
    # And skip compiling external sources altogether:
    exclude_external_sources = True,
)
