#pragma once
#include "knitter/task/graph.h"

namespace knitter {

enum TaskType {
    PLACEHOLDER_TASK = Node::PLACEHOLDER_WORK,
    STATIC_TASK = Node::STATIC_WORK,
    DYNAMIC_TASK = Node::DYNAMIC_WORK,
    CONDITION_TASK = Node::CONDITION_WORK,
    MODULE_TASK = Node::MODULE_WORK,
    EXIT_TASK = Node::EXIT_WORK,
    NUM_TASK_TYPES
};

inline const char* task_type_to_string(TaskType type) {
    const char* val;

    switch (type) {
        case PLACEHOLDER_TASK:
            val = "placeholder";
            break;
        case STATIC_TASK:
            val = "static";
            break;
        case DYNAMIC_TASK:
            val = "subflow";
            break;
        case CONDITION_TASK:
            val = "condition";
            break;
        case MODULE_TASK:
            val = "module";
            break;
        case EXIT_TASK:
            val = "exit";
            break;
        default:
            val = "undefined";
            break;
    }

    return val;
}

// ----------------------------------------------------------------------------
// Task Traits
// ----------------------------------------------------------------------------

// determines if a callable is a static task
// A static task is a callable object constructible from std::function<void()>.
// template <typename C>
// constexpr bool is_static_task_v = is_invocable_r_v<void, C> &&
//                                  !is_invocable_r_v<int, C>;

// determines if a callable is a dynamic task
// A dynamic task is a callable object constructible from
// std::function<void(Subflow&)>.
// template <typename C>
// constexpr bool is_dynamic_task_v = is_invocable_r_v<void, C, Subflow&>;

// determines if a callable is a condition task
// A condition task is a callable object constructible from
// std::function<int()>.
// template <typename C>
// constexpr bool is_condition_task_v = is_invocable_r_v<int, C>;

class Task {
public:
    Task() = default;
    Task(Node*);
    Task(const Task& other);

    Task& operator=(const Task&);
    Task& operator=(std::nullptr_t);
    bool operator==(const Task& rhs) const;
    bool operator!=(const Task& rhs) const;

    const std::string& name() const;
    Task& name(const std::string& name);

    size_t num_successors() const;
    size_t num_dependents() const;
    size_t num_strong_dependents() const;
    size_t num_weak_dependents() const;

    // assigns a static task
    // @param callable a callable object constructible from
    // std::function<void()>
    template <typename C>
    std::enable_if_t<is_invocable_r<void, C>::value && !is_invocable_r<int, C>::value, Task>& work(C&& callable);

    // assigns a dynamic task
    // @param  callable a callable object constructible from
    // std::function<void(Subflow&)>
    template <typename C>
    std::enable_if_t<is_invocable_r<void, C, Subflow&>::value, Task>& work(C&& callable);

    // assigns a condition task
    // @param callable a callable object constructible from std::function<int()>
    template <typename C>
    std::enable_if_t<is_invocable_r<int, C>::value, Task>& work(C&& callable);

    // creates a module task from a taskflow
    Task& composed_of(Taskflow& taskflow);

    template <typename... Ts>
    Task& precede(Ts&&... tasks);

    template <typename... Ts>
    Task& succeed(Ts&&... tasks);

    // resets the task handle to null
    void reset();

    // resets the associated work to a placeholder
    void reset_work();

    // queries if the task handle points to a task node
    bool empty() const;

    // queries if the task has a work assigned
    bool has_work() const;

    // applies an visitor callable to each successor of the task
    template <typename V>
    void for_each_successor(V&& visitor) const;

    // applies an visitor callable to each dependents of the task
    template <typename V>
    void for_each_dependent(V&& visitor) const;

    // obtains a hash value of the underlying node
    size_t hash_value() const;

    TaskType type() const;

    std::string dump() {
        return _node ? _node->dump() : "-";
    }

public:
    Node* _node{nullptr};
    std::unordered_map<std::string, std::string> _extra_info;

private:
    template <typename T>
    void _precede(T&&);

    template <typename T, typename... Rest>
    void _precede(T&&, Rest&&...);

    template <typename T>
    void _succeed(T&&);

    template <typename T, typename... Rest>
    void _succeed(T&&, Rest&&...);
};

template <typename... Ts>
Task& Task::precede(Ts&&... tasks) {
    _precede(std::forward<Ts>(tasks)...);
    return *this;
}

template <typename T>
void Task::_precede(T&& other) {
    _node->_precede(other._node);
}

template <typename T, typename... Ts>
void Task::_precede(T&& task, Ts&&... others) {
    _precede(std::forward<T>(task));
    _precede(std::forward<Ts>(others)...);
}

template <typename... Ts>
Task& Task::succeed(Ts&&... tasks) {
    _succeed(std::forward<Ts>(tasks)...);
    return *this;
}

template <typename T>
void Task::_succeed(T&& other) {
    other._node->_precede(_node);
}

template <typename T, typename... Ts>
void Task::_succeed(T&& task, Ts&&... others) {
    _succeed(std::forward<T>(task));
    _succeed(std::forward<Ts>(others)...);
}

template <typename V>
void Task::for_each_successor(V&& visitor) const {
    for (size_t i = 0; i < _node->_successors.size(); ++i) {
        visitor(Task(_node->_successors[i]));
    }
}

template <typename V>
void Task::for_each_dependent(V&& visitor) const {
    for (size_t i = 0; i < _node->_dependents.size(); ++i) {
        visitor(Task(_node->_dependents[i]));
    }
}

template <typename C>
std::enable_if_t<is_invocable_r<void, C>::value && !is_invocable_r<int, C>::value, Task>& Task::work(C&& c) {
    _node->_handle.emplace<Node::StaticWork>(std::forward<C>(c));
    return *this;
}

template <typename C>
std::enable_if_t<is_invocable_r<void, C, Subflow&>::value, Task>& Task::work(C&& c) {
    _node->_handle.emplace<Node::DynamicWork>(std::forward<C>(c));
    return *this;
}

template <typename C>
std::enable_if_t<is_invocable_r<int, C>::value, Task>& Task::work(C&& c) {
    _node->_handle.emplace<Node::ConditionWork>(std::forward<C>(c));
    return *this;
}

}  // end of namespace knitter

namespace std {
// hash specialization for std::hash<knitter::Task>
template <>
struct hash<knitter::Task> {
    auto operator()(const knitter::Task& task) const noexcept {
        return task.hash_value();
    }
};
}  // end of namespace std