#include <asmjit/x86.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "arch_framework/core/reflection.h"
#include "arch_framework/core/ast.h"
#include "arch_framework/core/ast/op.h"

TEST(PARSETest, TestSimpleObjBinaryOp) {
    {
        int32_t x = 1, y = 2, z;
        SimpleObjBinaryOp<std::plus<>> plus;
        auto f = plus.fun(vInt, vInt);
        f((int64_t)&x, (int64_t)&y, (int64_t)&z);
        ASSERT_EQ(z, x + y);
    }
    {
        double x = 1.0, y = 2.0, z;
        SimpleObjBinaryOp<std::plus<>> plus;
        auto f = plus.fun(vDouble, vDouble);
        f((int64_t)&x, (int64_t)&y, (int64_t)&z);
        ASSERT_EQ(z, x + y);
    }
    {
        double x = 1.0;
        std::vector<int64_t> y = {0, 1, 2, 3};
        std::vector<double> z;
        SimpleObjBinaryOp<std::plus<>> plus;
        auto f = plus.fun(vDouble, vLongs);
        f((int64_t)&x, (int64_t)&y, (int64_t)&z);
        for (size_t i = 0; i < y.size(); ++i) {
            ASSERT_EQ(z[i], x + y[i]);
        }
    }
}
