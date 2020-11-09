#include "knitter/task/graph.h"
#include <sstream>

namespace knitter {

ObjectPool<Node>& Graph::_node_pool() {
    static ObjectPool<Node> pool;
    return pool;
}

Graph::~Graph() {
    auto& np = _node_pool();
    for (auto node : _nodes) {
        // node->~Node();
        // np.deallocate(node);
        np.recycle(node);
    }
}

Graph::Graph(Graph&& other) : _nodes{std::move(other._nodes)} {
}

Graph& Graph::operator=(Graph&& other) {
    _nodes = std::move(other._nodes);
    return *this;
}

void Graph::clear() {
    auto& np = _node_pool();
    for (auto node : _nodes) {
        // node->~Node();
        // np.deallocate(node);
        np.recycle(node);
    }
    _nodes.clear();
}

void Graph::clear_detached() {
    auto mid = std::partition(_nodes.begin(), _nodes.end(),
                              [](Node* node) { return !(node->_has_state(Node::DETACHED)); });

    auto& np = _node_pool();
    for (auto itr = mid; itr != _nodes.end(); ++itr) {
        np.recycle(*itr);
    }
    _nodes.resize(std::distance(_nodes.begin(), mid));
}

void Graph::merge(Graph&& g) {
    for (auto n : g._nodes) {
        _nodes.push_back(n);
    }
    g._nodes.clear();
}

size_t Graph::size() const {
    return _nodes.size();
}

bool Graph::empty() const {
    return _nodes.empty();
}

std::string Graph::info() {
    std::ostringstream oss;
    oss << "nodes(" << _nodes.size() << "):[";
    size_t count = 0;
    for (auto& n : _nodes) {
        count++;
        if (!n)
            continue;
        std::string name = n->name();
        if (name.empty()) {
            oss << n;
        } else {
            oss << name;
        }
        if (count != _nodes.size()) {
            oss << " ";
        }
    }
    oss << "]";
    return oss.str();
}

}  // end of namespace knitter