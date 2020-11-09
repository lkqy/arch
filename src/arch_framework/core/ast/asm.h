#pragma once

#include <asmjit/x86.h>
#include <random>
#include <strstream>
#include "ast.h"
#include <cmath>
#include "arch_framework/util/hash/murmurhash/MurmurHash3.h"

bool load_instruction_op(AstNode& node, asmjit::x86::Compiler& cc, std::ostrstream& logger);
typedef int32_t (*Func)(int64_t, int64_t);
typedef void (*FUNC0)(int64_t);
typedef void (*FUNC1)(int64_t, int64_t);
typedef void (*FUNC2)(int64_t, int64_t, int64_t);
typedef void (*FUNC3)(int64_t, int64_t, int64_t, int64_t);

template <typename T>
class Context {
public:
    Context(std::shared_ptr<Ast<T>> a, Func f) : ast(a), func(f){};
    Context(std::shared_ptr<Context<T>>& ctx) : ast(ctx->ast), func(ctx->func){};
    void eval(T& t) {
        int64_t ctx = (int64_t)&t;
        int64_t tmp = (int64_t)&buffer;
        func(ctx, tmp);
    }
    void init() {
        // 初始化常量
        ast->dfs([&](AstNode& n) {
            char* p = buffer + n.data_offset;
            if (n.operator_type == kCONST) {
                // 常量初始化
                paser_const_value(n.value_type, p, n.result_varible);
            } else if (n.operator_type != kVAR && n.operator_type != kASIGN) {
                // 中间变量初始化
                if (n.value_type == vInts) {
                    auto q = new (p) std::vector<int32_t>();
                } else if (n.value_type == vLongs) {
                    auto q = new (p) std::vector<int64_t>();
                } else if (n.value_type == vFloats) {
                    auto q = new (p) std::vector<float>();
                } else if (n.value_type == vDoubles) {
                    auto q = new (p) std::vector<double>();
                } else if (n.value_type == vString) {
                    auto q = new (p) std::string();
                } else if (n.value_type == vStrings) {
                    auto q = new (p) std::vector<std::string>();
                }
            }
        });
    }

    ~Context() {
        ast->dfs([&](AstNode& n) {
            char* p = buffer + n.data_offset;
            if (n.operator_type != kVAR and *(int64_t*)p != 0 and n.operator_type != kASIGN) {
                if (n.value_type == vInts) {
                    auto q = (std::vector<int32_t>*)p;
                    q->~vector<int32_t>();
                } else if (n.value_type == vLongs) {
                    auto q = (std::vector<int64_t>*)p;
                    q->~vector<int64_t>();
                } else if (n.value_type == vFloats) {
                    auto q = (std::vector<float>*)p;
                    q->~vector<float>();
                } else if (n.value_type == vDoubles) {
                    auto q = (std::vector<double>*)p;
                    q->~vector<double>();
                } else if (n.value_type == vString) {
                    auto q = (std::string*)p;
                    q->~basic_string<char>();
                } else if (n.value_type == vStrings) {
                    auto q = (std::vector<std::string>*)p;
                    q->~vector<std::string>();
                }
            }
        });
    }

protected:
    std::shared_ptr<Ast<T>> ast;
    Func func;
    char buffer[1024 * 16];
};

template <typename T>
class Jit {
public:
    Jit() {
        // 为避免同类型不同名称的std::function 内部共享static变量，将注册的名字作为模板参数，强制创建出不一样的static
        // 变量
        static char Double[] = "Double";
        regist_func<std::string, double, Double>(
                [](const std::string& v) -> double { return std::stod(v.c_str(), nullptr); });
        regist_func<int64_t, double, Double>([](const int64_t& v) -> double { return double(v); });
        regist_func<int32_t, double, Double>([](const int32_t& v) -> double { return double(v); });
        regist_func<float, double, Double>([](const float& v) -> double { return double(v); });
        regist_func<bool, double, Double>([](const bool& v) -> double { return double(v); });
        regist_func<double, double, Double>([](const double& v) -> double { return double(v); });

        static char Long[] = "Long";
        regist_func<std::string, int64_t, Long>(
                [](const std::string& v) -> int64_t { return std::stoll(v.c_str(), nullptr); });
        regist_func<int64_t, int64_t, Long>([](const int64_t& v) -> int64_t { return v; });
        regist_func<int32_t, int64_t, Long>([](const int32_t& v) -> int64_t { return v; });
        regist_func<double, int64_t, Long>([](const double& v) -> int64_t { return v; });
        regist_func<float, int64_t, Long>([](const float& v) -> int64_t { return v; });
        regist_func<bool, int64_t, Long>([](const bool& v) -> int64_t { return v; });

        static char Str[] = "Str";
        regist_func<int64_t, std::string, Str>([](const int64_t& v) -> std::string { return std::to_string(v); });
        regist_func<int32_t, std::string, Str>([](const int32_t& v) -> std::string { return std::to_string(v); });
        regist_func<double, std::string, Str>([](const double& v) -> std::string { return std::to_string(v); });
        regist_func<float, std::string, Str>([](const float& v) -> std::string { return std::to_string(v); });
        regist_func<bool, std::string, Str>([](const bool& v) -> std::string { return std::to_string(v); });
        regist_func<std::string, std::string, Str>([](const std::string& v) -> std::string { return v; });

        static char Log[] = "Log";
        regist_func<int64_t, double, Log>([](const int64_t& v) -> double { return std::log(v); });
        regist_func<int32_t, double, Log>([](const int32_t& v) -> double { return std::log(v); });
        regist_func<float, double, Log>([](const float& v) -> double { return std::log(v); });
        regist_func<double, double, Log>([](const double& v) -> double { return std::log(v); });

        static char Combine[] = "Combine";
        regist_func<std::string, std::string, std::string, Combine>(
                [](const std::string& a, const std::string& b) -> std::string { return a + b; });
        regist_func<std::string, std::string, std::string, std::string, Combine>(
                [](const std::string& a, const std::string& b, const std::string& c) -> std::string {
                    return a + b + c;
                });
        regist_func<int64_t, int64_t, std::string, Combine>(
                [](const int64_t& a, const int64_t& b) -> std::string { return std::to_tring(a) + std::to_string(b); });
        regist_func<int64_t, std::string, std::string, Combine>(
                [](const int64_t& a, const std::string& b) -> std::string { return std::to_tring(a) + b; });
        regist_func<std::string, int64_t, std::string, Combine>(
                [](const std::string& a, const int64_t& b) -> std::string { return a + std::to_string(b); });

        static char Mask[] = "Mask";
        regist_func<int32_t, int64_t, int64_t, int64_t, Mask>(
                [](const int32_t id, const int64_t& top_bit, const int64_t& v) -> int64_t {
                    uint64_t _id = id;
                    uint64_t _t = top_bit;
                    uint64_t _v = v;
                    uint64_t x = (_id & (0XFFFFFFFFFFFFFFFF - (1 << _t))) | (_v << _t);
                    return x;
                });
        regist_func<int64_t, int64_t, int64_t, int64_t, Mask>(
                [](const int64_t id, const int64_t& top_bit, const int64_t& v) -> int64_t {
                    uint64_t _id = id;
                    uint64_t _t = top_bit;
                    uint64_t _v = v;
                    uint64_t x = (_id & (0XFFFFFFFFFFFFFFFF - (1 << _t))) | (_v << _t);
                    return x;
                });
        regist_func<std::vector<int32_t>, int64_t, int64_t, std::vector<int64_t>, Mask>(
                [](const std::vector<int32_t> ids, const int64_t& top_bit, const int64_t& v) -> std::vector<int64_t> {
                    std::vector<int64_t> _ids(ids.size());
                    uint64_t _t = top_bit;
                    uint64_t _v = v;
                    for (size_t i = 0; i < ids.size(); ++i) {
                        uint64_t _id = ids[i];
                        uint64_t x = (_id & (0XFFFFFFFFFFFFFFFF - (1 << _t))) | (_v << _t);
                        _ids[i] = x;
                    }
                    return _ids;
                });
        regist_func<std::vector<int64_t>, int64_t, int64_t, std::vector<int64_t>, Mask>(
                [](const std::vector<int64_t> ids, const int64_t& top_bit, const int64_t& v) -> std::vector<int64_t> {
                    std::vector<int64_t> _ids(ids.size());
                    uint64_t _t = top_bit;
                    uint64_t _v = v;
                    for (size_t i = 0; i < ids.size(); ++i) {
                        uint64_t _id = ids[i];
                        uint64_t x = (_id & (0XFFFFFFFFFFFFFFFF - (1 << _t))) | (_v << _t);
                        _ids[i] = x;
                    }
                    return _ids;
                });

        static char Append[] = "Append";
        regist_func<std::vector<double>, double, bool, Append>(
                [](const std::vector<double>& list, const double& a) -> bool {
                    auto _list = (std::vector<double>*)&list;
                    _list->push_back(a);
                    return true;
                });
        regist_func<std::vector<int64_t>, int64_t, bool, Append>(
                [](const std::vector<int64_t>& list, const int64_t& a) -> bool {
                    auto _list = (std::vector<int64_t>*)&list;
                    _list->push_back(a);
                    return true;
                });
        regist_func<std::vector<double>, std::vector<double>, bool, Append>(
                [](const std::vector<double>& list, const std::vector<double>& as) -> bool {
                    auto _list = (std::vector<double>*)&list;
                    for (auto a : as) {
                        _list->push_back(a);
                    }
                    return true;
                });
        regist_func<std::vector<int64_t>, std::vector<int64_t>, bool, Append>(
                [](const std::vector<int64_t>& list, const std::vector<int64_t>& as) -> bool {
                    auto _list = (std::vector<int64_t>*)&list;
                    for (auto a : as) {
                        _list->push_back(a);
                    }
                    return true;
                });

        static char Hash[] = "Hash";
        regist_func<int64_t, int64_t, Hash>([](const int64_t& v) -> int64_t { return v; });
        regist_func<int32_t, int64_t, Hash>([](const int32_t& v) -> int64_t { return v; });
        regist_func<std::string, int64_t, Hash>([](const std::string& v) -> int64_t {
            int64_t h[2];
            MurmurHash3_x64_128(v.c_str(), v.length(), 0xc70f6907u, &h);
            return h[0];
        });

        static char Rand[] = "Rand";
        regist_func<int64_t, Rand>([]() -> int64_t { return std::rand(); });

        regist_func<int64_t, int64_t, int64_t, Rand>([](const int64_t& a, const int64_t& b) -> int64_t {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(a, b);
            auto x = distrib(gen);
            return x;
        });
        regist_func<double, double, double, Rand>([](const double& a, const double& b) -> double {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> distrib(a, b);
            auto x = distrib(gen);
            return x;
        });

        static char IF[] = "IF";
        regist_func<bool, int64_t, int64_t, int64_t, IF>(
                [](const bool& f, const int64_t& v1, const int64_t& v2) -> int64_t { return f ? v1 : v2; });
        regist_func<bool, int32_t, int32_t, int32_t, IF>(
                [](const bool& f, const int32_t& v1, const int32_t& v2) -> int32_t { return f ? v1 : v2; });
        regist_func<bool, float, float, float, IF>(
                [](const bool& f, const float& v1, const float& v2) -> float { return f ? v1 : v2; });
        regist_func<bool, double, double, double, IF>(
                [](const bool& f, const double& v1, const double& v2) -> double { return f ? v1 : v2; });
        regist_func<bool, bool, bool, bool, IF>(
                [](const bool& f, const bool& v1, const bool& v2) -> bool { return f ? v1 : v2; });
        regist_func<bool, std::string, std::string, std::string, IF>(

                [](const bool& f, const std::string& v1, const std::string& v2) -> std::string { return f ? v1 : v2; });
    }
    // 强类型匹配
    template <typename RetT, const char* str>
    void regist_func(std::function<RetT()> fn) {
        std::string new_key = std::string(str);
        // 必须要用值捕获，否则fn会析构
        func0map[new_key] = [=](AstNode& node) -> FUNC0 {
            // ToDo: 判断类型是否正确
            node.value_type = get_value_type(typeid(RetT));
            static std::function<RetT()> func = fn;
            return [](int64_t a) {
                auto _a = (RetT*)a;
                *_a = func();
            };
        };
    }
    template <typename ParamT, typename RetT, const char* str>
    void regist_func(std::function<RetT(const ParamT&)> fn) {
        std::string new_key = std::string(str) + ":" + get_type_name<ParamT>();
        // 必须要用值捕获，否则fn会析构
        func1map[new_key] = [=](AstNode& node) -> FUNC1 {
            // ToDo: 判断类型是否正确
            if (get_value_type(typeid(ParamT)) != node.nodes[0]->value_type) {
                return nullptr;
            }
            node.value_type = get_value_type(typeid(RetT));
            static std::function<RetT(const ParamT&)> func = fn;
            return [](int64_t a, int64_t b) {
                auto _a = (ParamT*)a;
                auto _b = (RetT*)b;
                *_b = func(*_a);
            };
        };
    }
    template <typename ParamT1, typename ParamT2, typename RetT, const char* str>
    void regist_func(std::function<RetT(const ParamT1&, const ParamT2&)> fn) {
        std::string new_key = std::string(str) + ":" + get_type_name<ParamT1>() + ":" + get_type_name<ParamT2>();
        func2map[new_key] = [=](AstNode& node) -> FUNC2 {
            // ToDo: 判断类型是否正确
            if (get_value_type(typeid(ParamT1)) != node.nodes[0]->value_type or
                get_value_type(typeid(ParamT2)) != node.nodes[1]->value_type) {
                return nullptr;
            }
            node.value_type = get_value_type(typeid(RetT));
            static std::function<RetT(const ParamT1&, const ParamT2&)> func = fn;
            return [](int64_t a, int64_t b, int64_t c) {
                auto _a = (ParamT1*)a;
                auto _b = (ParamT2*)b;
                auto _c = (RetT*)c;
                *_c = func(*_a, *_b);
            };
        };
    }
    template <typename ParamT1, typename ParamT2, typename ParamT3, typename RetT, const char* str>
    void regist_func(std::function<RetT(const ParamT1&, const ParamT2&, const ParamT3&)> fn) {
        std::string new_key = std::string(str) + ":" + get_type_name<ParamT1>() + ":" + get_type_name<ParamT2>() + ":" +
                              get_type_name<ParamT3>();
        // 必须要用值捕获，否则fn会析构
        func3map[new_key] = [=](AstNode& node) -> FUNC3 {
            // ToDo: 判断类型是否正确
            if (get_value_type(typeid(ParamT1)) != node.nodes[0]->value_type or
                get_value_type(typeid(ParamT2)) != node.nodes[1]->value_type or
                get_value_type(typeid(ParamT3)) != node.nodes[2]->value_type) {
                return nullptr;
            }
            node.value_type = get_value_type(typeid(RetT));
            static std::function<RetT(const ParamT1&, const ParamT2&, const ParamT3&)> func = fn;
            return [](int64_t a, int64_t b, int64_t c, int64_t d) {
                auto _a = (ParamT1*)a;
                auto _b = (ParamT2*)b;
                auto _c = (ParamT3*)c;
                auto _d = (RetT*)d;
                auto x = func(*_a, *_b, *_c);
                *_d = x;
            };
        };
    }

    std::shared_ptr<Context<T>> get(const std::string& exp) {
        logger.clear();
        auto x = _get(exp);
        if (x == nullptr) {
            logger.freeze(false);
            _logger = "[" + exp + "] is wrong: " + std::string(logger.str(), logger.pcount());
        }
        return x;
    }
    // 一个ctx的起始地址，一个是中间变量地址
    std::shared_ptr<Context<T>> _get(const std::string& exp) {
        auto it = exp_ctxs.find(exp);
        if (it != exp_ctxs.end()) {
            for (auto& ctx : it->second) {
                if (ctx.use_count() == 1) {
                    return ctx;
                }
            }
            auto ctx = std::make_shared<Context<T>>(it->second[0]);
            // Tip: 可能会创建过于频繁
            it->second.push_back(ctx);
            return ctx;
        }
        using namespace asmjit;
        code.init(rt.environment());
        code.setLogger(&asm_logger);

        x86::Compiler cc(&code);
        cc.addFunc(FuncSignatureT<void, int64_t, int64_t>(asmjit::CallConv::kIdFastCall));
        // 第一个参数是ctx地址，第二个参数是临时变量地址
        x86::Gp ctx_ptr = cc.newInt64("ctx_ptr");
        x86::Gp tmp_ptr = cc.newInt64("tmp_ptr");
        cc.setArg(0, ctx_ptr);
        cc.setArg(1, tmp_ptr);

        auto ast = std::make_shared<Ast<T>>();

        if (not ast->init(exp)) {
            // ToDo: expression is wrong
            logger << ast->error_log();
            return nullptr;
        }
        // 初始化数据地址
        size_t start = 0;
        ast->dfs([&](AstNode& n) {
            if (n.operator_type == kVAR) {
                n.data_pointer = ctx_ptr;
                n.data_offset = ast->offset(n.result_varible);
                n.value_type = ast->type(n.result_varible);
            } else if (n.operator_type == kASIGN) {
                n.data_pointer = n.nodes[0]->data_pointer;
                n.data_offset = n.nodes[0]->data_offset;
            } else {
                n.data_pointer = tmp_ptr;
                n.data_offset = start;
                start += 64;  // 每一个临时变量的空间大小
            }
        });

        // 初始化命令
        bool ret = ast->dfs_bool([&](AstNode& node) -> bool {
            if (node.operator_type == kFUNC0) {
                std::string func_name = node.result_varible;
                auto it = func0map.find(func_name);
                if (it == func0map.end()) {
                    logger << "[" << func_name << "] not exists. ";
                    return false;  // 函数不存在
                }
                auto ptr = it->second(node);
                if (ptr == nullptr) {
                    logger << "[" << func_name << "] params type is wrong. ";
                    // 参数不合法
                    return false;
                }
                asmjit::x86::Gp arg1 = cc.newUInt64("arg1");
                cc.mov(arg1, node.data_offset);
                cc.add(arg1, node.data_pointer);

                asmjit::InvokeNode* n;
                cc.invoke(&n, asmjit::imm((void*)ptr),
                          asmjit::FuncSignatureT<void, int64_t>(asmjit::CallConv::kIdFastCall));
                n->setArg(0, arg1);
            } else if (node.operator_type == kFUNC1) {
                auto n0 = node.nodes[0];
                std::string func_name = node.result_varible + ":" + get_type_name(n0->value_type);
                auto it = func1map.find(func_name);
                if (it == func1map.end()) {
                    logger << "[" << func_name << "]1 not exists. ";
                    return false;  // 函数不存在
                }
                auto ptr = it->second(node);
                if (ptr == nullptr) {
                    logger << "[" << func_name << "] params type is wrong. ";
                    return false;
                }
                asmjit::x86::Gp arg1 = cc.newUInt64("arg1");
                cc.mov(arg1, n0->data_offset);
                cc.add(arg1, n0->data_pointer);
                asmjit::x86::Gp arg2 = cc.newUInt64("arg2");
                cc.mov(arg2, node.data_offset);
                cc.add(arg2, node.data_pointer);

                asmjit::InvokeNode* n;
                cc.invoke(&n, asmjit::imm((void*)ptr),
                          asmjit::FuncSignatureT<void, int64_t, int64_t>(asmjit::CallConv::kIdFastCall));
                n->setArg(0, arg1);
                n->setArg(1, arg2);
            } else if (node.operator_type == kFUNC2) {
                auto n0 = node.nodes[0];
                auto n1 = node.nodes[1];
                std::string func_name =
                        node.result_varible + ":" + get_type_name(n0->value_type) + ":" + get_type_name(n1->value_type);
                auto it = func2map.find(func_name);
                if (it == func2map.end()) {
                    logger << "[" << func_name << "]2 not exists. ";
                    return false;  // 函数不存在
                }
                auto ptr = it->second(node);
                if (ptr == nullptr) {
                    logger << "[" << func_name << "] params type is wrong. ";
                    return false;
                }
                asmjit::x86::Gp arg1 = cc.newUInt64("arg1");
                cc.mov(arg1, n0->data_offset);
                cc.add(arg1, n0->data_pointer);
                asmjit::x86::Gp arg2 = cc.newUInt64("arg2");
                cc.mov(arg2, n1->data_offset);
                cc.add(arg2, n1->data_pointer);
                asmjit::x86::Gp arg3 = cc.newUInt64("arg3");
                cc.mov(arg3, node.data_offset);
                cc.add(arg3, node.data_pointer);

                asmjit::InvokeNode* n;
                cc.invoke(&n, asmjit::imm((void*)ptr),
                          asmjit::FuncSignatureT<void, int64_t, int64_t, int64_t>(asmjit::CallConv::kIdFastCall));
                n->setArg(0, arg1);
                n->setArg(1, arg2);
                n->setArg(2, arg3);
            } else if (node.operator_type == kFUNC3) {
                auto n0 = node.nodes[0];
                auto n1 = node.nodes[1];
                auto n2 = node.nodes[2];
                std::string func_name = node.result_varible + ":" + get_type_name(n0->value_type) + ":" +
                                        get_type_name(n1->value_type) + ":" + get_type_name(n2->value_type);
                auto it = func3map.find(func_name);
                if (it == func3map.end()) {
                    logger << "[" << func_name << "]3 not exists. ";
                    return false;  // 函数不存在
                }
                auto ptr = it->second(node);
                if (ptr == nullptr) {
                    logger << "[" << func_name << "] params type is wrong. ";
                    return false;
                }
                asmjit::x86::Gp arg1 = cc.newUInt64("arg1");
                cc.mov(arg1, n0->data_offset);
                cc.add(arg1, n0->data_pointer);
                asmjit::x86::Gp arg2 = cc.newUInt64("arg2");
                cc.mov(arg2, n1->data_offset);
                cc.add(arg2, n1->data_pointer);
                asmjit::x86::Gp arg3 = cc.newUInt64("arg3");
                cc.mov(arg3, n2->data_offset);
                cc.add(arg3, n2->data_pointer);
                asmjit::x86::Gp arg4 = cc.newUInt64("arg4");
                cc.mov(arg4, node.data_offset);
                cc.add(arg4, node.data_pointer);

                asmjit::InvokeNode* n;
                cc.invoke(&n, asmjit::imm((void*)ptr), asmjit::FuncSignatureT<void, int64_t, int64_t, int64_t, int64_t>(
                                                               asmjit::CallConv::kIdFastCall));
                n->setArg(0, arg1);
                n->setArg(1, arg2);
                n->setArg(2, arg3);
                n->setArg(3, arg4);
            } else {
                if (not load_instruction_op(node, cc, logger)) {
                    logger << "load instruction [" << node.operator_type << "] failed. ";
                    return false;
                }
            }
            return true;
        });
        if (not ret) {
            return nullptr;
        }
        cc.endFunc();
        cc.finalize();
        Func fn;
        // ast->print();

        if (rt.add(&fn, &code) == asmjit::kErrorOk) {
            exps[exp] = fn;
            // std::cout << "asmjit log:" << asm_logger.data() << "\n";
            auto ctx = std::make_shared<Context<T>>(ast, fn);
            exp_ctxs[exp].push_back(ctx);
            ctx->init();
            return ctx;
        } else {
            logger << "init asm failed. ";
        }
        return nullptr;
    }

    ~Jit() {
        for (auto it : exps) {
            // std::cout<<"free:"<<it.first<<"\n";
            rt.release(it.second);
        }
    }
    const std::string& error_log() {
        return _logger;
    }

private:
    std::unordered_map<std::string, std::function<FUNC0(AstNode& node)>> func0map;
    std::unordered_map<std::string, std::function<FUNC1(AstNode& node)>> func1map;
    std::unordered_map<std::string, std::function<FUNC2(AstNode& node)>> func2map;
    std::unordered_map<std::string, std::function<FUNC3(AstNode& node)>> func3map;
    std::unordered_map<std::string, Func> exps;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Context<T>>>> exp_ctxs;
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::StringLogger asm_logger;
    std::ostrstream logger;
    std::string _logger;
};
