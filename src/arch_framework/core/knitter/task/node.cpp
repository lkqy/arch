#include "knitter/task/graph.h"

namespace knitter {

Node::~Node() {
    // this is to avoid stack overflow
    if (_handle.index() == DYNAMIC_WORK) {
        auto& subgraph = nstd::get<DynamicWork>(_handle).subgraph;
        std::vector<Node*> nodes;
        std::move(subgraph._nodes.begin(), subgraph._nodes.end(), std::back_inserter(nodes));
        subgraph._nodes.clear();

        size_t i = 0;
        while (i < nodes.size()) {
            if (nodes[i]->_handle.index() == DYNAMIC_WORK) {
                auto& sbg = nstd::get<DynamicWork>(nodes[i]->_handle).subgraph;
                std::move(sbg._nodes.begin(), sbg._nodes.end(), std::back_inserter(nodes));
                sbg._nodes.clear();
            }
            ++i;
        }

        auto& np = Graph::_node_pool();
        for (i = 0; i < nodes.size(); ++i) {
            np.recycle(nodes[i]);
        }
    }
}

void Node::_precede(Node* v) {
    _successors.push_back(v);
    v->_dependents.push_back(this);
}

size_t Node::num_successors() const {
    return _successors.size();
}

size_t Node::num_dependents() const {
    return _dependents.size();
}

size_t Node::num_weak_dependents() const {
    return std::count_if(_dependents.begin(), _dependents.end(),
                         [](Node* node) { return node->_handle.index() == Node::CONDITION_WORK; });
}

size_t Node::num_strong_dependents() const {
    return std::count_if(_dependents.begin(), _dependents.end(),
                         [](Node* node) { return node->_handle.index() != Node::CONDITION_WORK; });
}

const std::string& Node::name() const {
    return _name;
}

Domain Node::domain() const {
    Domain domain;

    switch (_handle.index()) {
        case STATIC_WORK:
        case DYNAMIC_WORK:
        case CONDITION_WORK:
        case MODULE_WORK:
        case EXIT_WORK:
            domain = Domain::HOST;
            break;

        default:
            domain = Domain::HOST;
            break;
    }

    return domain;
}

void Node::_set_state(int flag) {
    _state |= flag;
}

void Node::_unset_state(int flag) {
    _state &= ~flag;
}

void Node::_clear_state() {
    _state = 0;
}

void Node::_set_up_join_counter() {
    int c = 0;

    for (auto p : _dependents) {
        if (p->_handle.index() == Node::CONDITION_WORK) {
            _set_state(Node::BRANCHED);
        } else {
            c++;
        }
    }

    _join_counter.store(c, std::memory_order_relaxed);
}

bool Node::_has_state(int flag) const {
    return _state & flag;
}

std::string Node::dump() {
    return exec_state_str(_exec_state) + "|" + handle_str(_handle.index()) + "|" + _name + "(" +
           (_parent ? _parent->name() : "-") + ")|" + std::to_string(_join_counter.load());
}

void Node::precede(Node* node) {
    _precede(node);
}

}  // end of namespace knitter