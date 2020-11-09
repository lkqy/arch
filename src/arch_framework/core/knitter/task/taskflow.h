#pragma once
#include <stack>
#include "knitter/task/flow_builder.h"
#include "knitter/task/topology.h"

namespace knitter {

// main entry to create a task dependency graph
class Taskflow : public FlowBuilder {
    friend class Topology;
    friend class Executor;
    friend class FlowBuilder;

    struct Dumper {
        std::stack<const Taskflow*> stack;
        std::unordered_set<const Taskflow*> visited;
    };

public:
    Taskflow(const std::string& name);

    Taskflow();

    virtual ~Taskflow();

    void dump(std::ostream& ostream) const;

    std::string dump() const;

    size_t num_tasks() const;

    bool empty() const;

    void name(const std::string&);

    const std::string& name() const;

    // clears the associated task dependency graph
    void clear();

    // applies an visitor callable to each task in the taskflow
    template <typename V>
    void for_each_task(V&& visitor) const;

private:
    std::string _name;
    Graph _graph;
    std::mutex _mtx;
    std::list<Topology> _topologies;

    void _dump(std::ostream&, const Taskflow*) const;
    void _dump(std::ostream&, const Node*, Dumper&) const;
    void _dump(std::ostream&, const Graph&, Dumper&) const;
};

template <typename V>
void Taskflow::for_each_task(V&& visitor) const {
    for (size_t i = 0; i < _graph._nodes.size(); ++i) {
        visitor(Task(_graph._nodes[i]));
    }
}

}  // end of namespace knitter