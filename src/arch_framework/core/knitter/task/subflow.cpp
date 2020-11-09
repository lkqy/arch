#include "knitter/task/subflow.h"
#include "knitter/task/executor.h"

namespace knitter {

Subflow::Subflow(Executor& executor, Node* parent, Graph& graph)
        : FlowBuilder{graph}, _executor{executor}, _parent{parent} {
}

bool Subflow::joinable() const {
    return _joinable;
}

void Subflow::join() {
    if (!_joinable) {
        KNITTER_THROW("subflow not joinable");
    }

    _executor._invoke_dynamic_work_external(_parent, _graph, false);
    _joinable = false;
}

void Subflow::detach() {
    if (!_joinable) {
        KNITTER_THROW("subflow already joined or detached");
    }

    _executor._invoke_dynamic_work_external(_parent, _graph, true);
    _joinable = false;
}

}  // end of namespace knitter