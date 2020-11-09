common_incs=[
    ".",
    "src",
]
common_cppflags = [
    "-std=c++17", "-O3", "-Wall", "-Wextra", "-Werror",
    "-Wno-unused-local-typedefs", "-Wno-unused-function", "-Wno-unused-parameter",
    "-Wno-unused-variable", "-Wno-comment",
]

common_deps = [
    "cpp3rdlib/gtest:1.6.0@//cpp3rdlib/gtest:gtest",
    "cpp3rdlib/asmjit:master@//cpp3rdlib/asmjit:asmjit",
    "data/knitter:master@//data/knitter:knitter",
]

cc_library(
    name="arch_framework",
    srcs=[
        "src/arch_framework/*.cpp",
        "src/arch_framework/*/*.cpp",
        "src/arch_framework/*/*/*.cpp",
    ],
    incs=common_incs,
    deps=common_deps,
    custom_deps={
              'x86_64-gcc830': common_deps,
    },
    extra_cppflags=common_cppflags,
#    extra_linkflags=["-lrt"],
    export_incs=["src"],
    link_all_symbols=True,
    bundle_path="lib",
)

cc_test(
    name="arch_framework_test",
    srcs=[
        "test/*/*.cpp",
        "test/*/*/*.cpp",
    ],
    incs=common_incs,
    deps=common_deps+[
        ":arch_framework",
    ],
    testdata=[
        "conf/dsl/model_feature.conf"
    ],
    extra_cppflags=common_cppflags,
    extra_linkflags=["-lrt"],
    bundle_path="lib",
)
