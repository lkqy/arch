#pragma once
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstring>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <stack>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "knitter/nstd/variant.h"

namespace knitter {

// Macro to check whether a class has a member function
#define TF_DEFINE_HAS_MEMBER(member_name)                                     \
    template <typename T>                                                     \
    class has_member_##member_name {                                          \
        typedef char yes_type;                                                \
        typedef long no_type;                                                 \
        template <typename U>                                                 \
        static yes_type test(decltype(&U::member_name));                      \
        template <typename U>                                                 \
        static no_type test(...);                                             \
                                                                              \
    public:                                                                   \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type); \
    }

#define TF_HAS_MEMBER(class_, member_name) has_member_##member_name<class_>::value

// Struct: dependent_false
template <typename... T>
struct dependent_false {
    static constexpr bool value = false;
};

// template <typename... T>
// constexpr auto dependent_false_v = dependent_false<T...>::value;

//-----------------------------------------------------------------------------
// Move-On-Copy
//-----------------------------------------------------------------------------

// Struct: MoC
template <typename T>
struct MoC {
    MoC(T&& rhs) : object(std::move(rhs)) {
    }
    MoC(const MoC& other) : object(std::move(other.object)) {
    }

    T& get() {
        return object;
    }

    mutable T object;
};

template <typename T>
auto make_moc(T&& m) {
    return MoC<T>(std::forward<T>(m));
}

//-----------------------------------------------------------------------------
// Functors.
//-----------------------------------------------------------------------------

//// Overloadded.
// template <typename... Ts>
// struct Functors : Ts... {
//  using Ts::operator()... ;
//};
//
// template <typename... Ts>
// Functors(Ts...) -> Functors<Ts...>;

// ----------------------------------------------------------------------------
// callable traits
// ----------------------------------------------------------------------------

template <typename F, typename... Args>
struct is_invocable : std::is_constructible<std::function<void(Args...)>,
                                            std::reference_wrapper<typename std::remove_reference<F>::type>> {};

// template <typename F, typename... Args>
// constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

template <typename R, typename F, typename... Args>
struct is_invocable_r : std::is_constructible<std::function<R(Args...)>,
                                              std::reference_wrapper<typename std::remove_reference<F>::type>> {};

// FIXME: not work on gcc4.9.2
template <typename R, typename F, typename... Args>
using is_invocable_r_v = typename is_invocable_r<R, F, Args...>::value;

// ----------------------------------------------------------------------------
// Function Traits
// reference: https://github.com/ros2/rclcpp
// ----------------------------------------------------------------------------

template <typename T>
struct tuple_tail;

template <typename Head, typename... Tail>
struct tuple_tail<std::tuple<Head, Tail...>> {
    using type = std::tuple<Tail...>;
};

// std::function
template <typename F>
struct function_traits {
    using arguments =
            typename tuple_tail<typename function_traits<decltype(&F::operator())>::argument_tuple_type>::type;

    static constexpr size_t arity = std::tuple_size<arguments>::value;

    template <size_t N>
    struct argument {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = std::tuple_element_t<N, arguments>;
    };

    template <size_t N>
    using argument_t = typename argument<N>::type;

    using return_type = typename function_traits<decltype(&F::operator())>::return_type;
};

// Free functions
template <typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_tuple_type = std::tuple<Args...>;

    static constexpr size_t arity = sizeof...(Args);

    template <size_t N>
    struct argument {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = std::tuple_element_t<N, std::tuple<Args...>>;
    };

    template <size_t N>
    using argument_t = typename argument<N>::type;
};

// function pointer
template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};

// function reference
template <typename R, typename... Args>
struct function_traits<R (&)(Args...)> : function_traits<R(Args...)> {};

// immutable lambda
template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R(C&, Args...)> {};

// mutable lambda
template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(C&, Args...)> {};

// decay to the raw type
template <typename F>
struct function_traits<F&> : function_traits<F> {};

template <typename F>
struct function_traits<F&&> : function_traits<F> {};

// ----------------------------------------------------------------------------
// nstd::variant
// ----------------------------------------------------------------------------
template <typename T, typename>
struct get_index;

template <size_t I, typename... Ts>
struct get_index_impl {};

template <size_t I, typename T, typename... Ts>
struct get_index_impl<I, T, T, Ts...> : std::integral_constant<size_t, I> {};

template <size_t I, typename T, typename U, typename... Ts>
struct get_index_impl<I, T, U, Ts...> : get_index_impl<I + 1, T, Ts...> {};

template <typename T, typename... Ts>
struct get_index<T, nstd::variant<Ts...>> : get_index_impl<0, T, Ts...> {};

template <typename T, typename... Ts>
using get_index_v = typename get_index<T, Ts...>::value;

// ----------------------------------------------------------------------------
// is_pod
//-----------------------------------------------------------------------------
template <typename T>
struct is_pod {
    static const bool value = std::is_trivial<T>::value && std::is_standard_layout<T>::value;
};

template <typename T>
using is_pod_v = typename is_pod<T>::value;

}  // end of namespace knitter
