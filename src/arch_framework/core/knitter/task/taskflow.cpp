#include "knitter/task/taskflow.h"

namespace knitter {

Taskflow::Taskflow(const std::string& name) : FlowBuilder{_graph}, _name{name} {
}

Taskflow::Taskflow() : FlowBuilder{_graph} {
}

Taskflow::~Taskflow() {
    assert(_topologies.empty());
}

void Taskflow::clear() {
    _graph.clear();
}

size_t Taskflow::num_tasks() const {
    return _graph.size();
}

bool Taskflow::empty() const {
    return _graph.empty();
}

void Taskflow::name(const std::string& name) {
    _name = name;
}

const std::string& Taskflow::name() const {
    return _name;
}

std::string Taskflow::dump() const {
    std::ostringstream oss;
    dump(oss);
    return oss.str();
}

void Taskflow::dump(std::ostream& os) const {
    os << "digraph Taskflow {\n";
    _dump(os, this);
    os << "}\n";
}

void Taskflow::_dump(std::ostream& os, const Taskflow* top) const {
    Dumper dumper;

    dumper.stack.push(top);
    dumper.visited.insert(top);

    while (!dumper.stack.empty()) {
        auto f = dumper.stack.top();
        dumper.stack.pop();

        os << "subgraph cluster_p" << f << " {\nlabel=\"Knitter: ";
        if (f->_name.empty())
            os << 'p' << f;
        else
            os << f->_name;
        os << "\";\n";
        _dump(os, f->_graph, dumper);
        os << "}\n";
    }
}

void Taskflow::_dump(std::ostream& os, const Node* node, Dumper& dumper) const {
    os << 'p' << node << "[label=\"";
    if (node->_name.empty())
        os << 'p' << node;
    else
        os << node->_name;
    os << "\" ";

    if (node->_exec_state == Node::ExecState::EXIT) {
        os << "fontcolor=white, style=filled, color=red ";
    } else if (node->_exec_state == Node::ExecState::IGNORE) {
        os << "fontcolor=white, style=filled, color=gray ";
    } else if (node->_exec_state == Node::ExecState::INIT) {
        os << "fontcolor=white, style=filled, color=blue ";
    }

    // shape for node
    switch (node->_handle.index()) {
        case Node::CONDITION_WORK:
            os << "shape=diamond color=black fillcolor=aquamarine style=filled";
            break;

        default:
            break;
    }

    os << "];\n";

    for (size_t s = 0; s < node->_successors.size(); ++s) {
        if (node->_handle.index() == Node::CONDITION_WORK) {
            // case edge is dashed
            os << 'p' << node << " -> p" << node->_successors[s] << " [style=dashed label=\"" << s << "\"];\n";
        } else {
            os << 'p' << node << " -> p" << node->_successors[s] << ";\n";
        }
    }

    // subflow join node
    if (node->_parent && node->_successors.size() == 0) {
        os << 'p' << node << " -> p" << node->_parent << ";\n";
    }

    switch (node->_handle.index()) {
        case Node::DYNAMIC_WORK: {
            auto& sbg = nstd::get<Node::DynamicWork>(node->_handle).subgraph;
            if (!sbg.empty()) {
                VLOG(4) << "sbg is not empty! node_name:" << node->_name << " sbg_size:" << sbg._nodes.size();
                os << "subgraph cluster_p" << node << " {\nlabel=\"Subflow: ";
                if (node->_name.empty())
                    os << 'p' << node;
                else
                    os << node->_name;

                os << "\";\n"
                   << "color=blue\n";
                _dump(os, sbg, dumper);
                os << "}\n";
            } else {
                VLOG(4) << "sbg is empty!";
            }
        } break;

        default:
            break;
    }
}

void Taskflow::_dump(std::ostream& os, const Graph& graph, Dumper& dumper) const {
    for (const auto& n : graph._nodes) {
        // regular task
        if (n->_handle.index() != Node::MODULE_WORK) {
            _dump(os, n, dumper);
        }
        // module task
        else {
            auto module = nstd::get<Node::ModuleWork>(n->_handle).module;

            os << 'p' << n << "[shape=box3d, color=blue, label=\"";
            if (n->_name.empty())
                os << n;
            else
                os << n->_name;
            os << " [Taskflow: ";
            if (module->_name.empty())
                os << 'p' << module;
            else
                os << module->_name;
            os << "]\"];\n";

            if (dumper.visited.find(module) == dumper.visited.end()) {
                dumper.visited.insert(module);
                dumper.stack.push(module);
            }

            for (const auto s : n->_successors) {
                os << 'p' << n << "->" << 'p' << s << ";\n";
            }
        }
    }
}

}  // end of namespace knitter