#include <asmjit/x86.h>
#include <stdio.h>
#include <unordered_map>
#include <gtest/gtest.h>
#include "arch_framework/core/reflection.h"

DEFINE_STRUCT(Point, (double)x, (double)y)

DEFINE_STRUCT(Rect, (Point)p1, (Point)p2, (uint32_t)color)

DEFINE_STRUCT(PP, (double)x, (double)y, (int)z, (long)m, (float)n)

TEST(PARSETest, TestReflection) {
    Rect rect{
            {1.2, 3.4}, {5.6, 7.8}, 12345678,
    };
    std::cout << dump_obj(rect) << "\n";

    PP pp{1.2, 3.4, 3, 1L, 1.0f};
    std::unordered_map<std::string, size_t> offs;
    for_each_address(pp, [&](auto&& field_name, char* address) {
        int offset = address - (char*)&pp;
        offs[field_name] = offset;
        std::cout << field_name << ":" << offset << "\n";
    });
}
