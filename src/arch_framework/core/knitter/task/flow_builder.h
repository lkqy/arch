#pragma once
#include <glog/logging.h>
#include <type_traits>
#include "knitter/task/task.h"
#include "knitter/task/task_view.h"
#include "knitter/utils/string_printf.h"

namespace knitter {

struct TraverseResult {
    int subgraphs = 0;
    int sources = 0;

    TraverseResult(int sg_num, int src_num) : subgraphs(sg_num), sources(src_num) {
    }
};

// FlowBuilder defines most of the operations for task composition
// Mind:
//  1) `emplace` is not thread-safe
//  2) if a flow contains dissociative graphs, all nodes belonging to it should
//     not be `ExitWork` and the `valid()` will return false in this condition.
class FlowBuilder {
    friend class Executor;

public:
    // creates a static task from a given callable object
    // @param callable a callable object constructible from
    // std::function<void()>
    // @return Task handle
    template <typename C>
    std::enable_if_t<is_invocable_r<void, C>::value && !is_invocable_r<int, C>::value, Task> emplace(C&& callable);

    // creates a dynamic task from a given callable object
    // @param callable a callable object constructible from
    // std::function<void(Subflow&)>
    // @return Task handle
    template <typename C>
    std::enable_if_t<is_invocable_r<void, C, Subflow&>::value, Task> emplace(C&& callable);

    // creates a condition task from a given callable object
    // @param callable a callable object constructible from std::function<int()>
    // @return Task handle
    template <typename C>
    std::enable_if_t<is_invocable_r<int, C>::value && !std::is_same<bool, typename std::result_of<C()>::type>::value,
                     Task>
    emplace(C&& callable);

    // creates a exit task from a given callable object
    // @param callable a callable object constructible from
    // std::function<bool()>
    // @return Task handle
    template <typename C>
    std::enable_if_t<std::is_same<bool, typename std::result_of<C()>::type>::value, Task> emplace(C&& callable);

    // creates multiple tasks from a list of callable objects
    // @param callables one or multiple callable objects constructible from each
    // task category
    //@return a Task handle
    template <typename... C, std::enable_if_t<(sizeof...(C) > 1), void>* = nullptr>
    auto emplace(C&&... callables);

    Task composed_of(Taskflow& taskflow);

    // @callable to each object obtained by dereferencing
    // every iterator in the range [beg, end). The range
    // is split into chunks of size @p chunk, where each of them
    // is processed by one Task.
    // The callable needs to accept a single argument, the object in the range.
    // @return a pair of Task handles to the beginning and the end of the graph
    template <typename I, typename C>
    std::pair<Task, Task> parallel_for(I beg, I end, C&& callable, size_t chunk = 1);

    // constructs a task dependency graph of integer index-based parallel_for
    // The task dependency graph applies a callable object to every index
    // in the range [beg, end) with a step size chunk by chunk
    // @return a pair of Task handles to the beginning and the end of the graph
    template <typename I, typename C, std::enable_if_t<std::is_integral<std::decay_t<I>>::value, void>* = nullptr>
    std::pair<Task, Task> parallel_for(I beg, I end, I step, C&& callable, size_t chunk = 1);

    // constructs a task dependency graph of floating index-based arallel_for
    template <typename I, typename C, std::enable_if_t<std::is_floating_point<std::decay_t<I>>::value, void>* = nullptr>
    std::pair<Task, Task> parallel_for(I beg, I end, I step, C&& callable, size_t chunk = 1);

    // construct a task dependency graph of parallel reduction
    // The task dependency graph reduces items in the range [beg, end) to a
    // single result.
    // @param beg    iterator to the beginning (inclusive)
    // @param end    iterator to the end (exclusive)
    // @param result reference variable to store the final result
    // @param bop    binary operator that will be applied in unspecified order
    // to the result of dereferencing the input iterator
    // @return a pair of Task handles to the beginning and the end of the graph
    template <typename I, typename T, typename B>
    std::pair<Task, Task> reduce(I beg, I end, T& result, B&& bop);

    // constructs a task dependency graph of parallel reduction through @std_min
    // The task dependency graph applies a parallel reduction
    // to find the minimum item in the range [beg, end) through @std_min
    // reduction.
    template <typename I, typename T>
    std::pair<Task, Task> reduce_min(I beg, I end, T& result);

    template <typename I, typename T>
    std::pair<Task, Task> reduce_max(I beg, I end, T& result);

    template <typename I, typename T, typename B, typename U>
    std::pair<Task, Task> transform_reduce(I beg, I end, T& result, B&& bop, U&& uop);

    template <typename I, typename T, typename B, typename P, typename U>
    std::pair<Task, Task> transform_reduce(I beg, I end, T& result, B&& bop1, P&& bop2, U&& uop);

    Task placeholder();

    void precede(Task A, Task B);

    // adds adjacent dependency links to a linear list of tasks
    void linearize(std::vector<Task>& tasks);

    // adds adjacent dependency links to a linear list of tasks
    void linearize(std::initializer_list<Task> tasks);

    // adds dependency links from one task A to many tasks
    void broadcast(Task A, std::vector<Task>& others);

    // adds dependency links from one task A to many tasks
    void broadcast(Task A, std::initializer_list<Task> others);

    // adds dependency links from many tasks to one task A
    void succeed(std::vector<Task>& others, Task A);

    // adds dependency links from many tasks to one task A
    void succeed(std::initializer_list<Task> others, Task A);

    // 前提：有向无环图
    TraverseResult traverse(bool deep = false);

    bool has_exit_node();

    bool is_adjusted();

    std::string info();

protected:
    FlowBuilder(Graph& graph);

    // associated graph object
    Graph& _graph;

private:
    bool _has_exit_node = false;
    bool _is_adjusted = false;

private:
    template <typename L>
    void _linearize(L&);

    bool check_link(const std::unordered_set<Node*>& a, const std::unordered_set<Node*>& b);

    void traverse_node(Node* n, std::unordered_set<Node*>& holder);
};

template <typename... C, std::enable_if_t<(sizeof...(C) > 1), void>*>
auto FlowBuilder::emplace(C&&... cs) {
    return std::make_tuple(emplace(std::forward<C>(cs))...);
}

template <typename C>
std::enable_if_t<is_invocable_r<void, C>::value && !is_invocable_r<int, C>::value, Task> FlowBuilder::emplace(C&& c) {
    auto n = _graph.emplace_back(nstd::in_place_type_t<Node::StaticWork>{}, std::forward<C>(c));
    VLOG(4) << "flow emplace work, handle_index:" << n->_handle.index();
    return Task(n);
}

template <typename C>
std::enable_if_t<is_invocable_r<void, C, Subflow&>::value, Task> FlowBuilder::emplace(C&& c) {
    auto n = _graph.emplace_back(nstd::in_place_type_t<Node::DynamicWork>{}, std::forward<C>(c));
    VLOG(4) << "flow emplace work, handle_index:" << n->_handle.index();
    return Task(n);
}

template <typename C>
std::enable_if_t<is_invocable_r<int, C>::value && !std::is_same<bool, typename std::result_of<C()>::type>::value, Task>
FlowBuilder::emplace(C&& c) {
    auto n = _graph.emplace_back(nstd::in_place_type_t<Node::ConditionWork>{}, std::forward<C>(c));
    VLOG(4) << "flow emplace work, handle_index:" << n->_handle.index();
    return Task(n);
}

template <typename C>
std::enable_if_t<std::is_same<bool, typename std::result_of<C()>::type>::value, Task> FlowBuilder::emplace(C&& c) {
    auto n = _graph.emplace_back(nstd::in_place_type_t<Node::ExitWork>{}, std::forward<C>(c));
    VLOG(4) << "flow emplace work, handle_index:" << n->_handle.index();
    _has_exit_node = true;
    return Task(n);
}

template <typename I, typename C>
std::pair<Task, Task> FlowBuilder::parallel_for(I beg, I end, C&& c, size_t chunk) {
    auto S = placeholder();
    auto T = placeholder();

    // default partition equals to the worker count
    if (chunk == 0) {
        chunk = 1;
    }

    size_t remain = std::distance(beg, end);

    while (beg != end) {
        auto e = beg;

        auto x = std::min(remain, chunk);
        std::advance(e, x);
        remain -= x;

        // Create a task
        auto task = emplace([beg, e, c]() mutable { std::for_each(beg, e, c); });

        S.precede(task);
        task.precede(T);

        // adjust the pointer
        beg = e;
    }

    // special case
    if (S.num_successors() == 0) {
        S.precede(T);
    }

    return std::make_pair(S, T);
}

template <typename I, typename C, std::enable_if_t<std::is_integral<std::decay_t<I>>::value, void>*>
std::pair<Task, Task> FlowBuilder::parallel_for(I beg, I end, I s, C&& c, size_t chunk) {
    if ((s == 0) || (beg < end && s <= 0) || (beg > end && s >= 0)) {
        KNITTER_THROW("invalid range [", beg, ", ", end, ") with step size ", s);
    }

    // source and target
    auto source = placeholder();
    auto target = placeholder();

    if (chunk == 0) {
        chunk = 1;
    }

    // positive case
    if (beg < end) {
        while (beg != end) {
            auto o = static_cast<I>(chunk) * s;
            auto e = std::min(beg + o, end);
            auto task = emplace([=]() mutable {
                for (auto i = beg; i < e; i += s) {
                    c(i);
                }
            });
            source.precede(task);
            task.precede(target);
            beg = e;
        }
    }
    // negative case
    else if (beg > end) {
        while (beg != end) {
            auto o = static_cast<I>(chunk) * s;
            auto e = std::max(beg + o, end);
            auto task = emplace([=]() mutable {
                for (auto i = beg; i > e; i += s) {
                    c(i);
                }
            });
            source.precede(task);
            task.precede(target);
            beg = e;
        }
    }

    if (source.num_successors() == 0) {
        source.precede(target);
    }

    return std::make_pair(source, target);
}

template <typename I, typename C, std::enable_if_t<std::is_floating_point<std::decay_t<I>>::value, void>*>
std::pair<Task, Task> FlowBuilder::parallel_for(I beg, I end, I s, C&& c, size_t chunk) {
    if ((s == 0) || (beg < end && s <= 0) || (beg > end && s >= 0)) {
        KNITTER_THROW("invalid range [", beg, ", ", end, ") with step size ", s);
    }

    // source and target
    auto source = placeholder();
    auto target = placeholder();

    if (chunk == 0) {
        chunk = 1;
    }

    // positive case
    if (beg < end) {
        size_t N = 0;
        I b = beg;
        for (I e = beg; e < end; e += s) {
            if (++N == chunk) {
                auto task = emplace([=]() mutable {
                    for (size_t i = 0; i < N; ++i, b += s) {
                        c(b);
                    }
                });
                source.precede(task);
                task.precede(target);
                N = 0;
                b = e;
            }
        }

        if (N) {
            auto task = emplace([=]() mutable {
                for (size_t i = 0; i < N; ++i, b += s) {
                    c(b);
                }
            });
            source.precede(task);
            task.precede(target);
        }
    } else if (beg > end) {
        size_t N = 0;
        I b = beg;
        for (I e = beg; e > end; e += s) {
            if (++N == chunk) {
                auto task = emplace([=]() mutable {
                    for (size_t i = 0; i < N; ++i, b += s) {
                        c(b);
                    }
                });
                source.precede(task);
                task.precede(target);
                N = 0;
                b = e;
            }
        }

        if (N) {
            auto task = emplace([=]() mutable {
                for (size_t i = 0; i < N; ++i, b += s) {
                    c(b);
                }
            });
            source.precede(task);
            task.precede(target);
        }
    }

    if (source.num_successors() == 0) {
        source.precede(target);
    }

    return std::make_pair(source, target);
}

template <typename I, typename T>
std::pair<Task, Task> FlowBuilder::reduce_min(I beg, I end, T& result) {
    return reduce(beg, end, result, [](const auto& l, const auto& r) { return std::min(l, r); });
}

template <typename I, typename T>
std::pair<Task, Task> FlowBuilder::reduce_max(I beg, I end, T& result) {
    return reduce(beg, end, result, [](const auto& l, const auto& r) { return std::max(l, r); });
}

template <typename I, typename T, typename B, typename U>
std::pair<Task, Task> FlowBuilder::transform_reduce(I beg, I end, T& result, B&& bop, U&& uop) {
    // Even partition
    size_t d = std::distance(beg, end);
    size_t w = std::max(unsigned{1}, std::thread::hardware_concurrency());
    size_t g = std::max((d + w - 1) / w, size_t{2});

    auto source = placeholder();
    auto target = placeholder();

    // std::vector<std::future<T>> futures;
    auto g_results = std::make_unique<T[]>(w);
    size_t id{0};

    size_t remain = d;

    while (beg != end) {
        auto e = beg;

        size_t x = std::min(remain, g);
        std::advance(e, x);
        remain -= x;

        // Create a task
        auto task = emplace([ beg, e, bop, uop, res = &(g_results[id]) ]() mutable {
            *res = uop(*beg);
            for (++beg; beg != e; ++beg) {
                *res = bop(std::move(*res), uop(*beg));
            }
        });

        source.precede(task);
        task.precede(target);

        // adjust the pointer
        beg = e;
        id++;
    }

    // target synchronizer
    target.work([&result, bop, res = make_moc(std::move(g_results)), w = id ]() {
        for (auto i = 0u; i < w; i++) {
            result = bop(std::move(result), res.object[i]);
        }
    });

    return std::make_pair(source, target);
}

template <typename I, typename T, typename B, typename P, typename U>
std::pair<Task, Task> FlowBuilder::transform_reduce(I beg, I end, T& result, B&& bop, P&& pop, U&& uop) {
    // Even partition
    size_t d = std::distance(beg, end);
    size_t w = std::max(unsigned{1}, std::thread::hardware_concurrency());
    size_t g = std::max((d + w - 1) / w, size_t{2});

    auto source = placeholder();
    auto target = placeholder();

    auto g_results = std::make_unique<T[]>(w);

    size_t id{0};
    size_t remain = d;

    while (beg != end) {
        auto e = beg;

        size_t x = std::min(remain, g);
        std::advance(e, x);
        remain -= x;

        // Create a task
        auto task = emplace([ beg, e, uop, pop, res = &g_results[id] ]() mutable {
            *res = uop(*beg);
            for (++beg; beg != e; ++beg) {
                *res = pop(std::move(*res), *beg);
            }
        });
        source.precede(task);
        task.precede(target);

        // adjust the pointer
        beg = e;
        id++;
    }

    // target synchronizer
    target.work([&result, bop, g_results = make_moc(std::move(g_results)), w = id ]() {
        for (auto i = 0u; i < w; i++) {
            result = bop(std::move(result), std::move(g_results.object[i]));
        }
    });

    return std::make_pair(source, target);
}

template <typename L>
void FlowBuilder::_linearize(L& keys) {
    auto itr = keys.begin();
    auto end = keys.end();

    if (itr == end) {
        return;
    }

    auto nxt = itr;

    for (++nxt; nxt != end; ++nxt, ++itr) {
        itr->_node->_precede(nxt->_node);
    }
}

template <typename I, typename T, typename B>
std::pair<Task, Task> FlowBuilder::reduce(I beg, I end, T& result, B&& op) {
    size_t d = std::distance(beg, end);
    size_t w = std::max(unsigned{1}, std::thread::hardware_concurrency());
    size_t g = std::max((d + w - 1) / w, size_t{2});

    auto source = placeholder();
    auto target = placeholder();

    // T* g_results = static_cast<T*>(malloc(sizeof(T)*w));
    auto g_results = std::make_unique<T[]>(w);
    // std::vector<std::future<T>> futures;

    size_t id{0};
    size_t remain = d;

    while (beg != end) {
        auto e = beg;

        size_t x = std::min(remain, g);
        std::advance(e, x);
        remain -= x;

        // Create a task
        // auto [task, future] = emplace([beg, e, op] () mutable {
        auto task = emplace([ beg, e, op, res = &g_results[id] ]() mutable {
            *res = *beg;
            for (++beg; beg != e; ++beg) {
                *res = op(std::move(*res), *beg);
            }
        });
        source.precede(task);
        task.precede(target);
        // futures.push_back(std::move(future));

        // adjust the pointer
        beg = e;
        id++;
    }

    target.work([ g_results = make_moc(std::move(g_results)), &result, op, w = id ]() {
        for (auto i = 0u; i < w; i++) {
            result = op(std::move(result), g_results.object[i]);
        }
    });

    return std::make_pair(source, target);
}

}  // end of namespace knitter
