#pragma once
#include "knitter/task/task.h"

namespace knitter {

class TaskView {
public:
    TaskView() = default;
    TaskView(Node*);
    TaskView(const Task& task);
    TaskView(const TaskView& other);

    TaskView& operator=(const TaskView& other);
    TaskView& operator=(const Task& other);
    TaskView& operator=(std::nullptr_t);
    bool operator==(const TaskView&) const;
    bool operator!=(const TaskView&) const;

    const std::string& name() const;

    size_t num_successors() const;

    size_t num_dependents() const;

    size_t num_strong_dependents() const;

    size_t num_weak_dependents() const;

    void reset();

    bool empty() const;

    TaskType type() const;

    template <typename V>
    void for_each_successor(V&& visitor) const;

    template <typename V>
    void for_each_dependent(V&& visitor) const;

private:
    Node* _node{nullptr};
};

template <typename V>
void TaskView::for_each_successor(V&& visitor) const {
    for (size_t i = 0; i < _node->_successors.size(); ++i) {
        visitor(TaskView(_node->_successors[i]));
    }
}

template <typename V>
void TaskView::for_each_dependent(V&& visitor) const {
    for (size_t i = 0; i < _node->_dependents.size(); ++i) {
        visitor(TaskView(_node->_dependents[i]));
    }
}

}  // end of namespace knitter
