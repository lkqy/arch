#pragma once

#include <asmjit/x86.h>
#include "ast.h"

bool load_instruction_op(AstNode& node, asmjit::x86::Compiler& cc);
typedef int32_t (*Func)(int64_t, int64_t);
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
    // 强类型匹配
    template <typename ParamT, typename RetT>
    void regist_func(const std::string& key, std::function<RetT(const ParamT&)> fn) {
        // 必须要用值捕获，否则fn会析构
        func1map[key] = [=](AstNode& node) -> FUNC1 {
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
    template <typename ParamT1, typename ParamT2, typename RetT>
    void regist_func(const std::string& key, std::function<RetT(const ParamT1&, const ParamT2&)> fn) {
        // 必须要用值捕获，否则fn会析构
        func2map[key] = [=](AstNode& node) -> FUNC2 {
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
    template <typename ParamT1, typename ParamT2, typename ParamT3, typename RetT>
    void regist_func(const std::string& key, std::function<RetT(const ParamT1&, const ParamT2&, const ParamT3&)> fn) {
        // 必须要用值捕获，否则fn会析构
        func3map[key] = [=](AstNode& node) -> FUNC3 {
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
                *_d = func(*_a, *_b, *_c);
            };
        };
    }

    // 一个ctx的起始地址，一个是中间变量地址
    std::shared_ptr<Context<T>> get(const std::string& exp) {
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
        code.setLogger(&logger);

        x86::Compiler cc(&code);
        cc.addFunc(FuncSignatureT<void, int64_t, int64_t>(asmjit::CallConv::kIdFastCall));
        // 第一个参数是ctx地址，第二个参数是临时变量地址
        x86::Gp ctx_ptr = cc.newInt64("ctx_ptr");
        x86::Gp tmp_ptr = cc.newInt64("tmp_ptr");
        cc.setArg(0, ctx_ptr);
        cc.setArg(1, tmp_ptr);

        auto ast = std::make_shared<Ast<T>>();

        if (ast->init(exp)) {
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
                    start += 24;
                }
            });

            // 初始化命令
            ast->dfs([&](AstNode& node) {
                if (node.operator_type == kFUNC1) {
                    auto it = func1map.find(node.result_varible);
                    if (it == func1map.end()) {
                        return false;  // 函数不存在
                    }
                    auto ptr = it->second(node);
                    if (ptr == nullptr) {
                        // 参数不合法
                        return false;
                    }
                    auto n0 = node.nodes[0];
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
                    auto it = func2map.find(node.result_varible);
                    if (it == func2map.end()) {
                        return false;  // 函数不存在
                    }
                    auto ptr = it->second(node);
                    if (ptr == nullptr) {
                        // 参数不合法
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
                    auto it = func3map.find(node.result_varible);
                    if (it == func3map.end()) {
                        return false;  // 函数不存在
                    }
                    auto ptr = it->second(node);
                    if (ptr == nullptr) {
                        // 参数不合法
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
                    cc.invoke(&n, asmjit::imm((void*)ptr),
                              asmjit::FuncSignatureT<void, int64_t, int64_t, int64_t, int64_t>(
                                      asmjit::CallConv::kIdFastCall));
                    n->setArg(0, arg1);
                    n->setArg(1, arg2);
                    n->setArg(2, arg3);
                    n->setArg(3, arg4);
                } else {
                    load_instruction_op(node, cc);
                }
                return true;
            });
        }
        cc.endFunc();
        cc.finalize();
        Func fn;
        //ast->print();

        if (rt.add(&fn, &code) == asmjit::kErrorOk) {
            exps[exp] = fn;
            // std::cout << "asmjit log:" << logger.data() << "\n";
            auto ctx = std::make_shared<Context<T>>(ast, fn);
            exp_ctxs[exp].push_back(ctx);
            return ctx;
        }
        return nullptr;
    }

    ~Jit() {
        for (auto it : exps) {
            std::cout << "free:" << it.first << "\n";
            rt.release(it.second);
        }
    }

private:
    std::unordered_map<std::string, std::function<FUNC1(AstNode& node)>> func1map;
    std::unordered_map<std::string, std::function<FUNC2(AstNode& node)>> func2map;
    std::unordered_map<std::string, std::function<FUNC3(AstNode& node)>> func3map;
    std::unordered_map<std::string, Func> exps;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Context<T>>>> exp_ctxs;
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::StringLogger logger;
};
