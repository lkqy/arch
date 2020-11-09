#include "knitter/task/task_view.h"

namespace knitter {

TaskView::TaskView(Node* node) : _node{node} {
}

TaskView::TaskView(const TaskView& rhs) : _node{rhs._node} {
}

TaskView::TaskView(const Task& task) : _node{task._node} {
}

TaskView& TaskView::operator=(const TaskView& rhs) {
    _node = rhs._node;
    return *this;
}

TaskView& TaskView::operator=(const Task& rhs) {
    _node = rhs._node;
    return *this;
}

TaskView& TaskView::operator=(std::nullptr_t ptr) {
    _node = ptr;
    return *this;
}

const std::string& TaskView::name() const {
    return _node->_name;
}

size_t TaskView::num_dependents() const {
    return _node->num_dependents();
}

size_t TaskView::num_strong_dependents() const {
    return _node->num_strong_dependents();
}

size_t TaskView::num_weak_dependents() const {
    return _node->num_weak_dependents();
}

size_t TaskView::num_successors() const {
    return _node->num_successors();
}

void TaskView::reset() {
    _node = nullptr;
}

bool TaskView::empty() const {
    return _node == nullptr;
}

TaskType TaskView::type() const {
    return static_cast<TaskType>(_node->_handle.index());
}

bool TaskView::operator==(const TaskView& rhs) const {
    return _node == rhs._node;
}

bool TaskView::operator!=(const TaskView& rhs) const {
    return _node != rhs._node;
}

}  // end of namespace knitter