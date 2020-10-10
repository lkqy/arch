#include <asmjit/x86.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "arch_framework/core/reflection.h"
#include "arch_framework/core/ast.h"
#include "arch_framework/core/ast/asm.h"
#include "arch_framework/util/simdjson/simdjson.h"

DEFINE_STRUCT(Point, (double)x, (double)y, (std::string)z)

DEFINE_STRUCT(Points, (std::vector<double>)x, (std::vector<double>)y)

TEST(PARSETest, TestCtx) {
    {
        std::string exp = "x=((x / y + y + y + x + x + (x + x + x + y + x + x)) + (y + x + y + x + y + x + x) + x + y)";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        ASSERT_TRUE(ctx != nullptr);
        ctx->init();
        Point p = {1.0, 3.0};
        ctx->eval(p);
        double x = 1.0;
        double y = 3.0;
        double z = (x / y + y + y + x + x + (x + x + x + y + x + x)) + (y + x + y + x + y + x + x) + x + y;
        ASSERT_EQ(p.x, z);
    }
    {
        Points p = {{1.0, 2.0, 3.0}, {-1.0, -2.0, -3.0f}};
        std::string exp = "x = x + y";
        Jit<Points> jit;
        auto ctx = jit.get(exp);
        ctx->init();
        ctx->eval(p);
        ASSERT_TRUE(p.x.size() == 3);
        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(p.x[i], 0);
        }
    }
    {
        std::string exp = "x=10.0";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0};
        ctx->init();
        ctx->eval(p);
        ASSERT_EQ(p.x, 10.0);
    }
    {
        std::string exp = "z='abc'";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->init();
        ctx->eval(p);
        ASSERT_TRUE(p.z == "abc");
    }
    {
        std::string exp = "x=y>10";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->init();
        ctx->eval(p);
        ASSERT_EQ(p.x, 0.0);
    }
    {
        std::string exp = "x=Rand(1.0)";
        Jit<Point> jit;
        jit.regist_func<double, double>("Rand", [](const double& r)->double {
                return r * 100.0;
        });
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->init();
        ctx->eval(p);
        ASSERT_EQ(p.x, 100.0);
    }
    {
        std::string exp = "x=Rand(1.0, 2.0)";
        Jit<Point> jit;
        jit.regist_func<double, double, double>("Rand", [](const double& a, const double& b)->double {
                return a * 100.0 + b;
        });
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->init();
        ctx->eval(p);
        ASSERT_EQ(p.x, 102.0);
    }
    {
        // 重复拿到ctx
        std::string exp = "x=10.0";
        Jit<Point> jit;
        auto ctx0 = jit.get(exp);
        {
            auto ctx1 = jit.get(exp);
        }
        {
            auto ctx1 = jit.get(exp);
        }
    }
}

TEST(PARSETest, TestPerformance) {
    {
        std::string exp = "((x / y + y + y + x + x + (x + x + x + y + x + x)) + (y + x + y + x + y + x + x) + x + y)";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        ASSERT_TRUE(ctx != nullptr);
        ctx->init();
        Point p = {1.0, 3.0};
        for (size_t i = 0; i < 1000000; ++i) {
            ctx->eval(p);
        }
        ASSERT_EQ(p.x, 1);
    }
}
TEST(PARSETest, TestPerformances) {
    {
        std::string exp = "((x / y + y + y + x + x + (x + x + x + y + x + x)) + (y + x + y + x + y + x + x) + x + y)";
        Jit<Points> jit;
        auto ctx = jit.get(exp);
        ASSERT_TRUE(ctx != nullptr);
        ctx->init();
        Points p = {{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                    {3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0}};
        for (size_t i = 0; i < 1000000; ++i) {
            ctx->eval(p);
        }
        // ASSERT_EQ(p.x, 1);
    }
}
TEST(PARSETest, TestSimdJson) {
    simdjson::dom::parser parser;
}
