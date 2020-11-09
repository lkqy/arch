#pragma once

namespace knitter {

class Topology {
    friend class Taskflow;
    friend class Executor;

public:
    template <typename P, typename C>
    Topology(Taskflow&, P&&, C&&);

    void reset_exit_invoked(bool v) {
        _exit_invoked.store(v);
    }

private:
    Taskflow& _taskflow;

    std::promise<int> _promise;

    PassiveVector<Node*> _sources;

    std::function<bool()> _pred;
    std::function<void()> _call;

    std::atomic<size_t> _join_counter{0};
    std::atomic<bool> _exit_invoked{false};
};

template <typename P, typename C>
inline Topology::Topology(Taskflow& tf, P&& p, C&& c)
        : _taskflow(tf), _pred{std::forward<P>(p)}, _call{std::forward<C>(c)} {
}

}  // end of namespace knitter