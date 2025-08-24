# For now, have sv parsing and 2d graphics seperate
cc_library (
    name = "sv_core",
    srcs = [
        "src/common.cc",
        "src/cst.cc",
        "src/symbol_table.cc",
    ],
    hdrs = [
        "lib/vec.h",
        "lib/common.h",
        "lib/sv.h",
        "lib/symbol_table.h",
        "lib/cst.h",
    ],
    deps = [
        "@bazel_tools//tools/cpp/runfiles", # to find runfiles portably
        "@nlohmann_json//:json",
    ],
    includes = ["lib"],
    visibility = ["//visibility:public"],
    data = ["@verible//verible/verilog/tools/syntax:verible-verilog-syntax"],  # bundle the tool
)

cc_binary(
    name = "sv_cst_test",
    srcs = [
        "src/sv_cst_test.cc",
    ],
    deps = [
        ":sv_core",
    ],
)

# 2D graphics
# TODO: Find a better, more automated way of packaging this in the future
cc_library(
    name = "sdl2_system",
    # let code `#include <SDL.h>` work:
    copts = ["-I/usr/include/SDL2"],
    # link the system lib; Bazel passes this through to the linker
    linkopts = ["-lSDL2"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "user_config",
    hdrs = ["include/config/SkUserConfig.h"],
    deps = [
        ":sv_core",
    ],
    includes = ["include/config"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "graphics",
    hdrs = [
        "lib/sv_colorizer.h",
        "lib/graphics.h"
    ],
    srcs = [
        "src/sv_colorizer.cc",
        "src/graphics.cc"
    ],
    deps = [
        "@skia//:core",
        "@skia//:png_encode_codec",
        "@skia//:fontmgr_fontconfig",
        "@skia//:fontmgr_empty_freetype",
        ":sdl2_system",
    ],
    includes = ["lib"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "skia_demo",
    srcs = ["src/skia_demo.cpp"],
    deps = [
        ":graphics"
    ]
)

# compile_commands.json generation
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_compile_commands",
    # Only index your C++ targets (add more as you like).
    targets = {
        "//:sv_cst_test": "--cxxopt=-std=gnu++17",
        "//:skia_demo": "--cxxopt=-std=gnu++17",
    },
    # Skip headers from external repos (avoids the Abseil header action).
    exclude_headers = "external",
    # And skip compiling external sources altogether:
    exclude_external_sources = True,
)
