cc_binary(
  name = "sv_cst_test",
  srcs = ["sv_cst_test.cc"],
  deps = [
    "@bazel_tools//tools/cpp/runfiles", # to find runfiles portably
    "@nlohmann_json//:json",
  ],
  data = ["@verible//verible/verilog/tools/syntax:verible-verilog-syntax"],  # bundle the tool
)
