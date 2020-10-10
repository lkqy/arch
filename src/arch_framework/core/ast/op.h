#pragma once
#include <vector>
#include <iostream>
#include <unordered_set>

template <typename OP>
struct SimpleObjBinaryOp {
    bool support(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        // int, long, float, double, bool, ints, longs, floats, doubles, bools
        if (a >= 1 and a <= 10 and b >= 1 and b <= 10) {
            return true;
        }
        return false;
    }
    auto fun(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        auto id = ((a - 1) * 10 + b - 1);
        return OPS[id];
    };
    auto result_type(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        bool has_vec = (a >= 6 and a <= 10) or (b >= 6 and b <= 10);
        if (a >= 6 and a <= 10)
            a -= 5;
        if (b >= 6 and b <= 10)
            b -= 5;
        std::vector<int> ids = {
                0, 1, 3, 3, 0, 1, 1, 3, 3, 1, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 0, 1, 2, 3, 4,
        };
        auto id = ids[a * 5 + b];
        if (has_vec) {
            id += 5;
        }
        return static_cast<ValueType>(id);
    }
    std::unordered_map<int, void(*)(int64_t, int64_t, int64_t)> OPS = {
        {0, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (int32_t*)b;
                auto _c = (int32_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {1, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (int64_t*)b;
                auto _c = (int64_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {2, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (float*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {3, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (double*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {4, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (bool*)b;
                auto _c = (int32_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {5, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {6, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {7, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {8, [](int64_t a, int64_t b, int64_t c) {
                                                    OP op;
                auto _a = (int32_t*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {9, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int32_t*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {10, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (int32_t*)b;
                auto _c = (int64_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {11, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (int64_t*)b;
                auto _c = (int64_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {12, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (float*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {13, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (double*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {14, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (bool*)b;
                auto _c = (int64_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {15, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {16, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {17, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {18, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {19, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (int64_t*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {20, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (int32_t*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {21, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (int64_t*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {22, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (float*)b;
                auto _c = (float*)c;
                *_c = op(*_a, *_b);
           }
        },
        {23, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (double*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {24, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (bool*)b;
                auto _c = (float*)c;
                *_c = op(*_a, *_b);
           }
        },
        {25, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {26, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {27, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {28, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {29, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (float*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {30, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (int32_t*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {31, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (int64_t*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {32, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (float*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {33, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (double*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {34, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (bool*)b;
                auto _c = (float*)c;
                *_c = op(*_a, *_b);
           }
        },
        {35, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {36, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {37, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {38, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {39, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (double*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {40, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (int32_t*)b;
                auto _c = (int32_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {41, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (int64_t*)b;
                auto _c = (int64_t*)c;
                *_c = op(*_a, *_b);
           }
        },
        {42, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (float*)b;
                auto _c = (float*)c;
                *_c = op(*_a, *_b);
           }
        },
        {43, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (double*)b;
                auto _c = (double*)c;
                *_c = op(*_a, *_b);
           }
        },
        {44, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (bool*)b;
                auto _c = (bool*)c;
                *_c = op(*_a, *_b);
           }
        },
        {45, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {46, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {47, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {48, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {49, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (bool*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                size_t s = _b->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(*_a, _b->at(i));
                }
           }
        },
        {50, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (int32_t*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {51, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (int64_t*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {52, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (float*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {53, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (double*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {54, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (bool*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {55, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {56, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {57, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {58, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {59, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {60, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (int32_t*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {61, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (int64_t*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {62, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (float*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {63, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (double*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {64, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (bool*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {65, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {66, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {67, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {68, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {69, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {70, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (int32_t*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {71, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (int64_t*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {72, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (float*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {73, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (double*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {74, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (bool*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {75, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {76, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {77, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {78, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {79, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<float>*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {80, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (int32_t*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {81, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (int64_t*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {82, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (float*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {83, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (double*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {84, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (bool*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {85, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {86, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {87, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {88, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {89, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<double>*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {90, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (int32_t*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {91, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (int64_t*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {92, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (float*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {93, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (double*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {94, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (bool*)b;
                auto _c = (std::vector<bool>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), *_b);
                }
           }
        },
        {95, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::vector<int32_t>*)b;
                auto _c = (std::vector<int32_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {96, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::vector<int64_t>*)b;
                auto _c = (std::vector<int64_t>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {97, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::vector<float>*)b;
                auto _c = (std::vector<float>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {98, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::vector<double>*)b;
                auto _c = (std::vector<double>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
        {99, [](int64_t a, int64_t b, int64_t c) {
        OP op;
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::vector<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                size_t s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = op(_a->at(i), _b->at(i));
                }
           }
        },
    };
};

template <typename OP>
struct StringObjBinaryOp {
    bool support(ValueType v1, ValueType v2) {
        if ((v1 == vString or v1 == vStrings) and (v2 == vString or v2 == vStrings)) {
            return true;
        }
        return false;
    }
    auto fun(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        auto id = (a - 11) * 2 + (b - 11);
        return OPS[id];
    }
    auto result_type(ValueType v1, ValueType v2) {
        if (v1 == vString and v1 == vString)
            return vBool;
        else if (v1 == vStrings or v2 == vStrings)
            return vBools;
        return vNull;
    }
    std::unordered_map<int, void (*)(int64_t, int64_t, int64_t)> OPS = {
            {0,
             [](int64_t a, int64_t b, int64_t c) {
                 OP op;
                 auto _a = (std::string*)a;
                 auto _b = (std::string*)b;
                 auto _c = (bool*)c;
                 *_c = op(*_a, *_b);
             }},
            {1,
             [](int64_t a, int64_t b, int64_t c) {
                 OP op;
                 auto _a = (std::string*)a;
                 auto _b = (std::vector<std::string>*)b;
                 auto _c = (std::vector<bool>*)c;
                 auto s = _b->size();
                 _c->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     *_c = op(*_a, _b->at(i));
                 }
             }},
            {2,
             [](int64_t a, int64_t b, int64_t c) {
                 OP op;
                 auto _a = (std::vector<std::string>*)a;
                 auto _b = (std::string*)b;
                 auto _c = (std::vector<std::string>*)c;
                 auto s = _a->size();
                 _c->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     *_c = op(_a->at(i), *_b);
                 }
             }},
            {3,
             [](int64_t a, int64_t b, int64_t c) {
                 OP op;
                 auto _a = (std::vector<std::string>*)a;
                 auto _b = (std::vector<std::string>*)b;
                 auto _c = (std::vector<std::string>*)c;
                 auto s = _a->size();
                 _c->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     *_c = op(_a->at(i), _b->at(i));
                 }
             }},
    };
};

struct SetObjBinaryOp {
    bool support(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        // int, long, float, double, bool, ints, longs, floats, doubles, bools
        if (a >= 1 and a <= 10 and b >= 15 and b <= 19) {
            return true;
        }
        return false;
    }
    auto fun(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        auto id = (a - 1) * 5 + b - 15;
        return OPS[id];
    }
    auto result_type(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        if (a >= 1 and a <= 5)
            return vBool;
        if (a >= 6 and a <= 10)
            return vBools;
        return vNull;
    }
    std::unordered_map<int, void(*)(int64_t, int64_t, int64_t)> OPS = {
        {0, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int32_t*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {1, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int32_t*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {2, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int32_t*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {3, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int32_t*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {4, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int32_t*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {5, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int64_t*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {6, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int64_t*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {7, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int64_t*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {8, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int64_t*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {9, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (int64_t*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {10, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (float*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {11, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (float*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {12, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (float*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {13, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (float*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {14, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (float*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {15, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (double*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {16, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (double*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {17, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (double*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {18, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (double*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {19, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (double*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {20, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (bool*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {21, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (bool*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {22, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (bool*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {23, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (bool*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {24, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (bool*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (bool*)c;
                *_c = _b->find(*_a) != _b->end();
           }
        },
        {25, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {26, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {27, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {28, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {29, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int32_t>*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {30, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {31, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {32, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {33, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {34, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<int64_t>*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {35, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<float>*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {36, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<float>*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {37, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<float>*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {38, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<float>*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {39, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<float>*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {40, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<double>*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {41, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<double>*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {42, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<double>*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {43, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<double>*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {44, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<double>*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {45, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::unordered_set<int32_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {46, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::unordered_set<int64_t>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {47, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::unordered_set<float>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {48, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::unordered_set<double>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
        {49, [](int64_t a, int64_t b, int64_t c) {
                auto _a = (std::vector<bool>*)a;
                auto _b = (std::unordered_set<bool>*)b;
                auto _c = (std::vector<bool>*)c;
                auto s = _a->size();
                _c->resize(s);
                for (size_t i = 0; i < s; ++i) {
                    _c->at(i) = _b->find(_a->at(i)) != _b->end();
                }
           }
        },
    };
};

struct StringSetObjBinaryOp {
    bool support(ValueType v1, ValueType v2) {
        if ((v1 == vString or v1 == vStrings) and v2 == vSetString) {
            return true;
        }
        return false;
    }
    auto fun(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        if (a == vString) {
            return OPS[0];
        }
        return OPS[1];
    }
    auto result_type(ValueType v1, ValueType v2) {
        if (v1 == vString)
            return vBool;
        if (v1 == vStrings)
            return vBools;
        return vNull;
    }
    std::unordered_map<int, void (*)(int64_t, int64_t, int64_t)> OPS = {
            {0,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::string*)a;
                 auto _b = (std::unordered_set<std::string>*)b;
                 auto _c = (bool*)c;
                 *_c = _b->find(*_a) != _b->end();
             }},
            {1,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<std::string>*)a;
                 auto _b = (std::unordered_set<std::string>*)b;
                 auto _c = (std::vector<bool>*)c;
                 auto s = _a->size();
                 _c->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _c->at(i) = _b->find(_a->at(i)) != _b->end();
                 }
             }},
    };
};

struct AssignOp {
    bool support(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        if ((a >= 1 and a <= 5) and (b >= 1 and b <= 5))
            return true;
        if ((a >= 6 and a <= 10) and (b >= 1 and b <= 10))
            return true;
        if (v1 == vString and v2 == vString)
            return true;
        if (v1 == vStrings and v2 == vStrings)
            return true;
        std::cout << "not support assign, a:" << a << ", b:" << b << "\n";
        return false;
    }
    auto fun(ValueType v1, ValueType v2) {
        auto a = static_cast<int>(v1);
        auto b = static_cast<int>(v2);
        int id = 0;
        if ((a >= 1 and a <= 5) and (b >= 1 and b <= 5)) {
            id = (a - 1) * 5 + b - 1;
        }
        if ((a >= 6 and a <= 10) and (b >= 1 and b <= 10)) {
            id = 25 + (a - 6) * 5 + b - 6;
        }
        if (v1 == vString) {
            id = 50;
        }
        if (v1 == vStrings) {
            id = 51;
        }
        std::cout << "assign id:" << id << "\n";
        return OPS[id];
    }
    auto result_type(ValueType v1, ValueType v2) {
        return v1;
    }
    std::unordered_map<int, void(*)(int64_t, int64_t, int64_t)> OPS = {
            {0,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int32_t*)a;
                 auto _b = (int32_t*)b;
                 *_a = *_b;
             }},
            {1,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int32_t*)a;
                 auto _b = (int64_t*)b;
                 *_a = *_b;
             }},
            {2,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int32_t*)a;
                 auto _b = (float*)b;
                 *_a = *_b;
             }},
            {3,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int32_t*)a;
                 auto _b = (double*)b;
                 *_a = *_b;
             }},
            {4,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int32_t*)a;
                 auto _b = (bool*)b;
                 *_a = *_b;
             }},
            {5,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int64_t*)a;
                 auto _b = (int32_t*)b;
                 *_a = *_b;
             }},
            {6,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int64_t*)a;
                 auto _b = (int64_t*)b;
                 *_a = *_b;
             }},
            {7,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int64_t*)a;
                 auto _b = (float*)b;
                 *_a = *_b;
             }},
            {8,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int64_t*)a;
                 auto _b = (double*)b;
                 *_a = *_b;
             }},
            {9,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (int64_t*)a;
                 auto _b = (bool*)b;
                 *_a = *_b;
             }},
            {10,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (float*)a;
                 auto _b = (int32_t*)b;
                 *_a = *_b;
             }},
            {11,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (float*)a;
                 auto _b = (int64_t*)b;
                 *_a = *_b;
             }},
            {12,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (float*)a;
                 auto _b = (float*)b;
                 *_a = *_b;
             }},
            {13,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (float*)a;
                 auto _b = (double*)b;
                 *_a = *_b;
             }},
            {14,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (float*)a;
                 auto _b = (bool*)b;
                 *_a = *_b;
             }},
            {15,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (double*)a;
                 auto _b = (int32_t*)b;
                 *_a = *_b;
             }},
            {16,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (double*)a;
                 auto _b = (int64_t*)b;
                 *_a = *_b;
             }},
            {17,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (double*)a;
                 auto _b = (float*)b;
                 *_a = *_b;
             }},
            {18,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (double*)a;
                 auto _b = (double*)b;
                 //std::cout<<a<<" , "<<b<<" "<<c<<" "<<*_a<<" "<<*_b<<"\n";
                 *_a = *_b;
             }},
            {19,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (double*)a;
                 auto _b = (bool*)b;
                 *_a = *_b;
             }},
            {20,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (bool*)a;
                 auto _b = (int32_t*)b;
                 *_a = *_b;
             }},
            {21,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (bool*)a;
                 auto _b = (int64_t*)b;
                 *_a = *_b;
             }},
            {22,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (bool*)a;
                 auto _b = (float*)b;
                 *_a = *_b;
             }},
            {23,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (bool*)a;
                 auto _b = (double*)b;
                 *_a = *_b;
             }},
            {24,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (bool*)a;
                 auto _b = (bool*)b;
                 *_a = *_b;
             }},
            {25,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int32_t>*)a;
                 auto _b = (std::vector<int32_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a[i] = _b[i];
                 }
             }},
            {26,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int32_t>*)a;
                 auto _b = (std::vector<int64_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {27,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int32_t>*)a;
                 auto _b = (std::vector<float>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {28,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int32_t>*)a;
                 auto _b = (std::vector<double>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {29,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int32_t>*)a;
                 auto _b = (std::vector<bool>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {30,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int64_t>*)a;
                 auto _b = (std::vector<int32_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {31,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int64_t>*)a;
                 auto _b = (std::vector<int64_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {32,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int64_t>*)a;
                 auto _b = (std::vector<float>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {33,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int64_t>*)a;
                 auto _b = (std::vector<double>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {34,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<int64_t>*)a;
                 auto _b = (std::vector<bool>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {35,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<float>*)a;
                 auto _b = (std::vector<int32_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {36,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<float>*)a;
                 auto _b = (std::vector<int64_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {37,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<float>*)a;
                 auto _b = (std::vector<float>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {38,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<float>*)a;
                 auto _b = (std::vector<double>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {39,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<float>*)a;
                 auto _b = (std::vector<bool>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {40,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<double>*)a;
                 auto _b = (std::vector<int32_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {41,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<double>*)a;
                 auto _b = (std::vector<int64_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {42,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<double>*)a;
                 auto _b = (std::vector<float>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {43,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<double>*)a;
                 auto _b = (std::vector<double>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {44,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<double>*)a;
                 auto _b = (std::vector<bool>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {45,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<bool>*)a;
                 auto _b = (std::vector<int32_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {46,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<bool>*)a;
                 auto _b = (std::vector<int64_t>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {47,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<bool>*)a;
                 auto _b = (std::vector<float>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {48,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<bool>*)a;
                 auto _b = (std::vector<double>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {49,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<bool>*)a;
                 auto _b = (std::vector<bool>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
            {50,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::string*)a;
                 auto _b = (std::string*)b;
                 *_a = *_b;
             }},
            {51,
             [](int64_t a, int64_t b, int64_t c) {
                 auto _a = (std::vector<std::string>*)a;
                 auto _b = (std::vector<std::string>*)b;
                 auto s = _b->size();
                 _a->resize(s);
                 for (size_t i = 0; i < s; ++i) {
                     _a->at(i) = _b->at(i);
                 }
             }},
    };
};
