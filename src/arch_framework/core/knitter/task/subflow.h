#pragma once
#include "knitter/task/flow_builder.h"

namespace knitter {

// class Subflow building methods of a subflow graph in dynamic tasking
class Subflow : public FlowBuilder {
    friend class Executor;

public:
    // enables the subflow to join its parent task
    // Performs an immediate action to join the subflow. Once the subflow is
    // joined, it is considered finished and you may not modify the subflow
    // anymore.
    void join();

    // enables the subflow to detach from its parent task
    // Performs an immediate action to detach the subflow. Once the subflow
    // isdetached, it is considered finished and you may not modify the subflow
    // anymore.
    void detach();

    // queries if the subflow is joinable
    // When a subflow is joined or detached, it becomes not joinable.
    bool joinable() const;

private:
    Subflow(Executor&, Node*, Graph&);

    Executor& _executor;
    Node* _parent;

    bool _joinable{true};
};

}  // end of namespace knitter