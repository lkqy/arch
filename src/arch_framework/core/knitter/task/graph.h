#pragma once
#include <glog/logging.h>
#include "knitter/nstd/variant.h"
#include "knitter/task/common.h"
#include "knitter/task/declarations.h"
#include "knitter/task/error.h"
#include "knitter/utility/object_pool.h"
#include "knitter/utility/os.h"
#include "knitter/utility/passive_vector.h"
#include "knitter/utility/singleton.h"
#include "knitter/utility/traits.h"
#include "knitter/utility/uuid.h"

namespace knitter {

class Graph {
    friend class Node;
    friend class Taskflow;
    friend class Executor;
    friend class FlowBuilder;

public:
    Graph() = default;
    Graph(const Graph&) = delete;
    Graph(Graph&&);

    ~Graph();

    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&);

    void clear();
    void clear_detached();
    void merge(Graph&&);

    bool empty() const;

    size_t size() const;

    template <typename... Args>
    Node* emplace_back(Args&&...);

    Node* emplace_back();

    std::string info();

private:
    static ObjectPool<Node>& _node_pool();

    std::vector<Node*> _nodes;
};

class Node {
    friend class Graph;
    friend class Task;
    friend class TaskView;
    friend class Topology;
    friend class Taskflow;
    friend class Executor;
    friend class FlowBuilder;
    friend class Subflow;

    TF_ENABLE_POOLABLE_ON_THIS;

    // state bit flag
    constexpr static int BRANCHED = 0x1;
    constexpr static int DETACHED = 0x2;

    // static work handle
    struct StaticWork {
        template <typename C>
        StaticWork(C&&);

        std::function<void()> work;
    };

    // dynamic work handle
    struct DynamicWork {
        template <typename C>
        DynamicWork(C&&);

        std::function<void(Subflow&)> work;
        Graph subgraph;
    };

    // condition work handle
    struct ConditionWork {
        template <typename C>
        ConditionWork(C&&);

        std::function<int()> work;
    };

    // module work handle
    struct ModuleWork {
        template <typename T>
        ModuleWork(T&&);

        Taskflow* module{nullptr};
    };

    // exit work handle
    struct ExitWork {
        template <typename T>
        ExitWork(T&&);

        std::function<bool()> work;
    };

    using handle_t = nstd::variant<nstd::monostate,  // placeholder
                                   StaticWork,       // static tasking
                                   DynamicWork,      // dynamic tasking
                                   ConditionWork,    // conditional tasking
                                   ModuleWork,       // composable tasking
                                   ExitWork          // exit taskflow tasking
                                   >;

    enum class ExecState : uint8_t { INIT = 0, DONE = 1, EXIT = 2, IGNORE = 3 };

public:
    constexpr static auto PLACEHOLDER_WORK = 0;
    constexpr static auto STATIC_WORK = 1;
    constexpr static auto DYNAMIC_WORK = 2;
    constexpr static auto CONDITION_WORK = 3;
    constexpr static auto MODULE_WORK = 4;
    constexpr static auto EXIT_WORK = 5;

    template <typename... Args>
    Node(Args&&... args);

    ~Node();

    size_t num_successors() const;
    size_t num_dependents() const;
    size_t num_strong_dependents() const;
    size_t num_weak_dependents() const;

    const std::string& name() const;

    Domain domain() const;

    std::string dump();

    void precede(Node* node);

    void reset_exec_state(const ExecState& es) {
        _exec_state = es;
    }

    static std::string handle_str(int handle_t_index) {
        switch (handle_t_index) {
            case 0:
                return "PLACEHOLDER_WORK";
            case 1:
                return "STATIC_WORK";
            case 2:
                return "DYNAMIC_WORK";
            case 3:
                return "CONDITION_WORK";
            case 4:
                return "MODULE_WORK";
            case 5:
                return "EXIT_WORK";
            default:
                return "UNKNOWN";
        }
    }

    static std::string exec_state_str(const ExecState& es) {
        switch (es) {
            case ExecState::INIT:
                return "INIT";
            case ExecState::DONE:
                return "DONE";
            case ExecState::EXIT:
                return "EXIT";
            case ExecState::IGNORE:
                return "IGNORE";
            default:
                return "UNKNOWN";
        }
    }

private:
    std::string _name;

    handle_t _handle;

    PassiveVector<Node*> _successors;
    PassiveVector<Node*> _dependents;

    Topology* _topology{nullptr};

    Node* _parent{nullptr};

    int _state{0};

    std::atomic<size_t> _join_counter{0};

    ExecState _exec_state = ExecState::INIT;

private:
    void _precede(Node*);
    void _set_state(int);
    void _unset_state(int);
    void _clear_state();
    void _set_up_join_counter();

    bool _has_state(int) const;
};

template <typename C>
Node::StaticWork::StaticWork(C&& c) : work{std::forward<C>(c)} {
}

template <typename C>
Node::DynamicWork::DynamicWork(C&& c) : work{std::forward<C>(c)} {
}

template <typename C>
Node::ConditionWork::ConditionWork(C&& c) : work{std::forward<C>(c)} {
}

template <typename T>
Node::ModuleWork::ModuleWork(T&& tf) : module{tf} {
}

template <typename C>
Node::ExitWork::ExitWork(C&& c) : work{std::forward<C>(c)} {
}

template <typename... Args>
Node::Node(Args&&... args) : _handle{std::forward<Args>(args)...} {
}

// ----------------------------------------------------------------------------
// Graph definition
// ----------------------------------------------------------------------------

// create a node from a give argument; constructor is called if necessary
template <typename... ArgsT>
Node* Graph::emplace_back(ArgsT&&... args) {
    // auto node = _node_pool().allocate();
    // new (node) Node(std::forward<ArgsT>(args)...);
    //_nodes.push_back(node);
    _nodes.push_back(_node_pool().animate(std::forward<ArgsT>(args)...));
    return _nodes.back();
}

// create a node from a give argument; constructor is called if necessary
inline Node* Graph::emplace_back() {
    // auto node = _node_pool().allocate();
    // new (node) Node();
    //_nodes.push_back(node);
    _nodes.push_back(_node_pool().animate());
    return _nodes.back();
}

}  // end of namespace knitter
