#include "knitter/task/task.h"

namespace knitter {

Task::Task(Node* node) : _node{node} {
}

Task::Task(const Task& rhs) : _node{rhs._node} {
}

Task& Task::composed_of(Taskflow& tf) {
    _node->_handle.emplace<Node::ModuleWork>(&tf);
    return *this;
}

Task& Task::operator=(const Task& rhs) {
    _node = rhs._node;
    return *this;
}

Task& Task::operator=(std::nullptr_t ptr) {
    _node = ptr;
    return *this;
}

bool Task::operator==(const Task& rhs) const {
    return _node == rhs._node;
}

bool Task::operator!=(const Task& rhs) const {
    return _node != rhs._node;
}

Task& Task::name(const std::string& name) {
    _node->_name = name;
    return *this;
}

void Task::reset() {
    _node = nullptr;
}

void Task::reset_work() {
    _node->_handle = nstd::monostate{};
}

const std::string& Task::name() const {
    return _node->_name;
}

size_t Task::num_dependents() const {
    return _node->num_dependents();
}

size_t Task::num_strong_dependents() const {
    return _node->num_strong_dependents();
}

size_t Task::num_weak_dependents() const {
    return _node->num_weak_dependents();
}

size_t Task::num_successors() const {
    return _node->num_successors();
}

bool Task::empty() const {
    return _node == nullptr;
}

bool Task::has_work() const {
    return _node ? _node->_handle.index() != 0 : false;
}

TaskType Task::type() const {
    return static_cast<TaskType>(_node->_handle.index());
}

size_t Task::hash_value() const {
    return std::hash<Node*>{}(_node);
}

}  // end of namespace knitter