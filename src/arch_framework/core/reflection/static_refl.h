/*************************************************************************
    > File Name: StaticRefl.hpp
    > Author: Netcan
    > Descripton: StaticRefl implement
    > Blog: https://netcan.github.io/
    > Mail: 1469709759@qq.com
    > Created Time: 2020-07-31 20:15
************************************************************************/
#pragma once

#include <type_traits>
#include <utility>
#include <cstddef>
#include <sstream>
#include "meta_macro.h"

#define FIELD_EACH(i, arg)                    \
    PAIR(arg);                                \
    template <typename T>                     \
    struct FIELD<T, i> {                      \
        T& obj;                               \
        FIELD(T& obj) : obj(obj) {            \
        }                                     \
        auto value() -> decltype(auto) {      \
            return (obj.STRIP(arg));          \
        }                                     \
        auto address() -> decltype(auto) {    \
            return (char*)&(obj.STRIP(arg));  \
        }                                     \
        auto type() -> decltype(auto) {       \
            return typeid(obj.STRIP(arg));    \
        }                                     \
        static constexpr const char* name() { \
            return STRING(STRIP(arg));        \
        }                                     \
    };

#define DEFINE_STRUCT(st, ...)                                                 \
    struct st {                                                                \
        template <typename, size_t>                                            \
        struct FIELD;                                                          \
        static constexpr size_t _field_count_ = GET_ARG_COUNT(__VA_ARGS__);    \
        PASTE(REPEAT_, GET_ARG_COUNT(__VA_ARGS__))(FIELD_EACH, 0, __VA_ARGS__) \
    };

template <typename T, typename F, size_t... Is>
inline constexpr void for_each(T&& obj, F&& f, std::index_sequence<Is...>) {
    using TDECAY = std::decay_t<T>;
    (void(f(typename TDECAY::template FIELD<TDECAY, Is>(obj).name(),
            typename TDECAY::template FIELD<TDECAY, Is>(obj).value())),
     ...);
}

template <typename T, typename F>
inline constexpr void for_each(T&& obj, F&& f) {
    for_each(std::forward<T>(obj), std::forward<F>(f), std::make_index_sequence<std::decay_t<T>::_field_count_>{});
}

template <typename T, typename F, size_t... Is>
inline constexpr void for_each_address(T&& obj, F&& f, std::index_sequence<Is...>) {
    using TDECAY = std::decay_t<T>;
    (void(f(typename TDECAY::template FIELD<TDECAY, Is>(obj).name(),
            typename TDECAY::template FIELD<TDECAY, Is>(obj).address())),
     ...);
}

template <typename T, typename F>
inline constexpr void for_each_address(T&& obj, F&& f) {
    for_each_address(std::forward<T>(obj), std::forward<F>(f),
                     std::make_index_sequence<std::decay_t<T>::_field_count_>{});
}

template <typename T, typename F, size_t... Is>
inline constexpr void for_each_type(T&& obj, F&& f, std::index_sequence<Is...>) {
    using TDECAY = std::decay_t<T>;
    (void(f(typename TDECAY::template FIELD<TDECAY, Is>(obj).name(),
            typename TDECAY::template FIELD<TDECAY, Is>(obj).type())),
     ...);
}

template <typename T, typename F>
inline constexpr void for_each_type(T&& obj, F&& f) {
    for_each_type(std::forward<T>(obj), std::forward<F>(f), std::make_index_sequence<std::decay_t<T>::_field_count_>{});
}

template <typename T>
void _dump_obj(T&& obj, std::stringstream& ss, const char* fieldName = "") {
    if
        constexpr(std::is_class_v<std::decay_t<T>>) {
            if (*fieldName) {
                ss << '"' << fieldName << "\":{";
            } else {
                std::cout << "{";
            }
            int count = 0;
            for_each(obj, [&](auto&& fieldName, auto&& value) {
                if (count > 0) {
                    ss << ",";
                }
                count += 1;
                _dump_obj(value, ss, fieldName);
            });
            ss << "}";
        }
    else {
        ss << '"' << fieldName << '"' << ": " << obj;
    }
}

template <typename T>
std::string dump_obj(T&& obj) {
    std::stringstream ss;
    _dump_obj(obj, ss);
    return ss.str();
}
