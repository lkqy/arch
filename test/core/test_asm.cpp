#include <asmjit/x86.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "arch_framework/core/reflection.h"
#include "arch_framework/core/ast.h"
#include "arch_framework/core/ast/asm.h"

DEFINE_STRUCT(Point, (double)x, (double)y, (std::string)z)

DEFINE_STRUCT(P1, (int64_t)x, (int64_t)y, (std::string)z)

DEFINE_STRUCT(P2, (std::vector<int64_t>)x, (std::vector<int64_t>)y)

DEFINE_STRUCT(Points, (std::vector<double>)x, (std::vector<double>)y)

TEST(PARSETest, ErrorExp) {
    std::vector<std::string> exps = {"a+b",
                                     "a+b+c",
                                     "And()",
                                     "f()",
                                     "Rand(1, 2, 3, 4)",
                                     "1.0 += 1",
                                     "1.0f > 1",
                                     "x * z",
                                     "x + z",
                                     "Rand(x, z)",
                                     "Rand(1, 1.0)",
                                     "XFF()",
                                     "X + y",
                                     "x + aa",
                                     "x & y",
                                     "x $ y",
                                     "x @",
                                     "x+",
                                     "_y",
                                     "%",
                                     "+",
                                     "(",
                                     ")",
                                     "(x"};
    for (auto e : exps) {
        Jit<Point> jit;
        auto ctx = jit.get(e);
        ASSERT_TRUE(ctx == nullptr);
        std::cout << jit.error_log() << "\n";
    }
}

TEST(PARSETest, TestCtx) {
    {
        std::string exp = "x=((x / y + y + y + x + x + (x + x + x + y + x + x)) + (y + x + y + x + y + x + x) + x + y)";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        ASSERT_TRUE(ctx != nullptr);
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
        ctx->eval(p);
        ASSERT_EQ(p.x, 10.0);
    }
    {
        std::string exp = "x=Long('123')";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_TRUE(p.x == 123);
    }
    {
        std::string exp = "z='abc'";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_TRUE(p.z == "abc");
    }
    {
        std::string exp = "y=Double('11.1')";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_EQ(p.y, 11.1);
    }
    {
        std::vector<std::string> exps = {
                "y=Rand()", "y=Rand(1, 1000)", "y=Rand(1.0, 1000.0)",
        };
        for (auto exp : exps) {
            Jit<Point> jit;
            auto ctx = jit.get(exp);
            if (ctx == nullptr) {
                std::cout << jit.error_log() << "\n";
            }
            ASSERT_TRUE(ctx != nullptr);
            Point p = {1.0, 3.0, ""};
            std::unordered_map<int, int> x;
            for (size_t i = 0; i < 10000; ++i) {
                ctx->eval(p);
                int c = p.y;
                x[c % 10] += 1;
            }
            for (auto it : x) {
                ASSERT_TRUE(it.second > 800 and it.second < 1200);
            }
        }
    }
    {
        std::string exp = "z=Str('11.1')";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_TRUE(p.z == "11.1");
    }
    {
        std::string exp = "z=Str(Double('11.1'))";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_TRUE(p.z == std::to_string(11.1));
    }
    {
        std::string exp = "y=Log(11)";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_EQ(p.y, std::log(11));
    }
    {
        std::string exp = "x=Hash('aa')";
        Jit<P1> jit;
        auto ctx = jit.get(exp);
        P1 p1 = {1L, 3L, ""};
        P1 p2 = {2L, 3L, ""};
        ctx->eval(p1);
        ctx->eval(p2);
        ASSERT_EQ(p1.x, p2.x);
        ASSERT_TRUE(p1.x != 0);
        ASSERT_TRUE(p1.x != 1);
        ASSERT_TRUE(p1.x != 2);
    }
    {
        std::string exp = "x=y>10";
        Jit<Point> jit;
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_EQ(p.x, 0.0);
    }
    {
        std::string exp = "Append(x, 1); Append(x, 2); Append(x, 3)";
        Jit<P2> jit;
        auto ctx = jit.get(exp);
        if (ctx == nullptr) {
            std::cout << jit.error_log() << "\n";
        }
        ASSERT_TRUE(ctx != nullptr);
        P2 p;
        ctx->eval(p);
        ASSERT_EQ(p.x.size(), 3);
        ASSERT_EQ(p.x[0], 1);
        ASSERT_EQ(p.x[1], 2);
        ASSERT_EQ(p.x[2], 3);
    }
    {
        std::string exp = "Append(x, 1); Append(x, 2); Append(x, 3); y=Mask(x, 5, 2)";
        Jit<P2> jit;
        auto ctx = jit.get(exp);
        if (ctx == nullptr) {
            std::cout << jit.error_log() << "\n";
        }
        ASSERT_TRUE(ctx != nullptr);
        P2 p;
        ctx->eval(p);
        ASSERT_EQ(p.x.size(), p.y.size());
        for (size_t i = 0; i < p.x.size(); ++i) {
            std::cout << "fid:" << p.x[i] << " " << p.y[i] << "\n";
        }
        ASSERT_EQ(p.x.size(), 3);
    }
    {
        std::string exp = "x=Xand(1.0)";
        Jit<Point> jit;
        static char Xand[] = "Xand";
        jit.regist_func<double, double, Xand>([](const double& r) -> double { return r * 100.0; });
        auto ctx = jit.get(exp);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_EQ(p.x, 100.0);
    }
    {
        std::string exp = "x=Xand(1.0, 2.0)";
        Jit<Point> jit;
        static char Xand[] = "Xand";
        jit.regist_func<double, double, double, Xand>(
                [](const double& a, const double& b) -> double { return a * 100.0 + b; });
        auto ctx = jit.get(exp);
        if (ctx == nullptr) {
            std::cout << jit.error_log() << "\n";
        }
        ASSERT_TRUE(ctx != nullptr);
        Point p = {1.0, 3.0, ""};
        ctx->eval(p);
        ASSERT_EQ(p.x, 102);
    }
    {
        // 重复拿到ctx
        std::string exp = "x=10.0";
        Jit<Point> jit;
        auto ctx0 = jit.get(exp);
        { auto ctx1 = jit.get(exp); }
        { auto ctx1 = jit.get(exp); }
    }
}

TEST(PARSETest, TestBasePerformance) {
    double x = 1.0;
    double y = 3.0;
    double ret;
    for (size_t i = 0; i < 10000000; ++i) {
        ret += ((x / y + y + y * x + x + (x + x * x + y / x + x)) + (y + x * y + x + y - x * x) + x * y);
    }

    ASSERT_TRUE(ret > 0);
}

TEST(PARSETest, TestPerformance) {
    std::string exp = "((x / y + y + y * x + x + (x + x * x + y / x + x)) + (y + x * y + x + y - x * x) + x * y)";
    Jit<Point> jit;
    auto ctx = jit.get(exp);
    ASSERT_TRUE(ctx != nullptr);
    Point p = {1.0, 3.0};
    for (size_t i = 0; i < 10000000; ++i) {
        ctx->eval(p);
    }
    ASSERT_EQ(p.x, 1);
}
TEST(PARSETest, TestVecPerformance) {
    std::string exp = "((x / y + y + y * x + x + (x + x * x + y / x + x)) + (y + x * y + x + y - x * x) + x * y)";
    Jit<Points> jit;
    auto ctx = jit.get(exp);
    ASSERT_TRUE(ctx != nullptr);
    Points p = {{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                {3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0}};
    for (size_t i = 0; i < 1000000; ++i) {
        ctx->eval(p);
    }
}
TEST(PARSETest, TestRapidJson) {
    // rapidjson::Document d;
    // nlohmann::json j;
    std::string s = "{\"a\":10}";
    //    auto j = nlohmann::json::parse(s);
    // d.Parse(d);
}
