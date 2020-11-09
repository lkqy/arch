#include "knitter/task/flow_builder.h"

namespace knitter {

FlowBuilder::FlowBuilder(Graph& graph) : _graph{graph} {
}

Task FlowBuilder::composed_of(Taskflow& taskflow) {
    auto node = _graph.emplace_back(nstd::in_place_type_t<Node::ModuleWork>{}, &taskflow);
    return Task(node);
}

void FlowBuilder::precede(Task from, Task to) {
    from._node->_precede(to._node);
}

void FlowBuilder::broadcast(Task from, std::vector<Task>& tos) {
    for (auto to : tos) {
        from.precede(to);
    }
}

void FlowBuilder::broadcast(Task from, std::initializer_list<Task> tos) {
    for (auto to : tos) {
        from.precede(to);
    }
}

void FlowBuilder::succeed(std::vector<Task>& froms, Task to) {
    for (auto from : froms) {
        to.succeed(from);
    }
}

void FlowBuilder::succeed(std::initializer_list<Task> froms, Task to) {
    for (auto from : froms) {
        to.succeed(from);
    }
}

Task FlowBuilder::placeholder() {
    auto node = _graph.emplace_back();
    return Task(node);
}

void FlowBuilder::linearize(std::vector<Task>& keys) {
    _linearize(keys);
}

void FlowBuilder::linearize(std::initializer_list<Task> keys) {
    _linearize(keys);
}

bool FlowBuilder::has_exit_node() {
    return _has_exit_node;
}

bool FlowBuilder::is_adjusted() {
    return _is_adjusted;
}

std::string FlowBuilder::info() {
    return knitter::utils::string_printf("has_exit_node:%s %s", _has_exit_node ? "true" : "false",
                                         _graph.info().c_str());
}

bool FlowBuilder::check_link(const std::unordered_set<Node*>& a, const std::unordered_set<Node*>& b) {
    for (const auto& item : a) {
        if (b.count(item) > 0) {
            return true;
        }
    }
    return false;
}

void FlowBuilder::traverse_node(Node* n, std::unordered_set<Node*>& holder) {
    holder.insert(n);

    if (n->num_successors() <= 0) {
        return;
    }

    for (auto& s : n->_successors) {
        traverse_node(s, holder);
    }
}

TraverseResult FlowBuilder::traverse(bool deep) {
    // FIXME: traverse recursively for deep
    // MODULE_WORK?
    size_t all_node_size = _graph._nodes.size();  // level one size
    if (all_node_size == 0) {
        return TraverseResult(0, 0);
    }

    std::vector<Node*> sources;
    for (const auto& node : _graph._nodes) {
        if (node->num_dependents() == 0) {
            sources.push_back(node);
        }
    }

    if (sources.size() <= 1) {
        return TraverseResult(sources.size(), sources.size());
    }

    std::list<std::unordered_set<Node*>> merged_subgraphs;
    std::vector<std::unordered_set<Node*>> subgraphs;
    for (size_t i = 0; i < sources.size(); i++) {
        const auto& s = sources[i];
        traverse_node(s, subgraphs[i]);
        merged_subgraphs.emplace_back(std::move(subgraphs[i]));
    }

    std::vector<std::pair<int, int>> links;
    for (size_t i = 0; i < subgraphs.size(); i++) {
        for (size_t j = i + 1; j < subgraphs.size(); j++) {
            bool has_link = check_link(subgraphs[i], subgraphs[j]);
            if (has_link) {
                links.emplace_back(i, j);
            }
        }
    }

    // FIXME
    return TraverseResult(links.size(), sources.size());
}

}  // end of namespace knitter
