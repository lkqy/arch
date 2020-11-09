#include "knitter/task/executor.h"
#include <glog/logging.h>

namespace knitter {

#define OUTPUT_EXECUTOR_STATS(x)                                                 \
    VLOG(4) << "executor stats(" << x << ") _num_actives:" << _num_actives[HOST] \
            << " _num_thieves:" << _num_thieves[HOST] << " _num_topologies:" << _num_topologies

Executor::Executor(size_t N, const std::string& thread_name)
        : _VICTIM_BEG{0},
          _VICTIM_END{N - 1},
          _MAX_STEALS{(N + 1) << 1},
          _MAX_YIELDS{100},
          _workers{N},
          _notifier{Notifier(N)} {
    if (N == 0) {
        KNITTER_THROW("no cpu workers to execute taskflows");
    }

    for (int i = 0; i < NUM_DOMAINS; ++i) {
        _num_actives[i].store(0, std::memory_order_relaxed);
        _num_thieves[i].store(0, std::memory_order_relaxed);
    }

    _spawn(N, HOST, thread_name);

    // instantite the default observer if requested
    _instantiate_tfprof();
}

Executor::~Executor() {
    // wait for all topologies to complete
    wait_for_all();

    // shut down the scheduler
    _done = true;

    for (int i = 0; i < NUM_DOMAINS; ++i) {
        _notifier[i].notify(true);
    }

    for (auto& t : _threads) {
        t.join();
    }

    // flush the default observer
    _flush_tfprof();
}

void Executor::_instantiate_tfprof() {
    // TF_OBSERVER_TYPE
    _tfprof = get_env("TF_ENABLE_PROFILER").empty() ? nullptr : make_observer<TFProfObserver>().get();
}

void Executor::_flush_tfprof() {
    if (_tfprof) {
        std::ostringstream fpath;
        fpath << get_env("TF_ENABLE_PROFILER") << _tfprof->_uuid << ".tfp";
        std::ofstream ofs(fpath.str());
        _tfprof->dump(ofs);
    }
}

size_t Executor::num_workers() const {
    return _workers.size();
}

size_t Executor::num_domains() const {
    return NUM_DOMAINS;
}

size_t Executor::num_topologies() const {
    return _num_topologies;
}

Executor::PerThread& Executor::_per_thread() const {
    thread_local PerThread pt;
    return pt;
}

int Executor::this_worker_id() const {
    auto worker = _per_thread().worker;
    return worker ? static_cast<int>(worker->id) : -1;
}

void Executor::_spawn(size_t N, Domain d, const std::string& thread_name) {
    auto id = _threads.size();

    for (size_t i = 0; i < N; ++i, ++id) {
        _workers[id].id = id;
        _workers[id].vtm = id;
        _workers[id].domain = d;
        _workers[id].executor = this;
        _workers[id].waiter = &_notifier[d]._waiters[i];

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%s.%zu", thread_name.c_str(), i);
        std::string tname = buffer;

        _threads.emplace_back(
                [this, tname](Worker& w) -> void {
                    pthread_setname_np(pthread_self(), tname.c_str());

                    PerThread& pt = _per_thread();
                    pt.worker = &w;
                    Node* t = nullptr;
                    // must use 1 as condition instead of !done
                    while (1) {
                        // execute the tasks.
                        _exploit_task(w, t);

                        // wait for tasks
                        if (_wait_for_task(w, t) == false) {
                            break;
                        }
                    }
                },
                std::ref(_workers[id]));
    }
}

void Executor::_explore_task(Worker& w, Node*& t) {
    assert(!t);
    const auto d = w.domain;
    size_t num_steals = 0;
    size_t num_yields = 0;

    std::uniform_int_distribution<size_t> rdvtm(_VICTIM_BEG, _VICTIM_END);
    do {
        t = (w.id == w.vtm) ? _wsq[d].steal() : _workers[w.vtm].wsq[d].steal();
        if (t) {
            break;
        }

        if (num_steals++ > _MAX_STEALS) {
            std::this_thread::yield();
            if (num_yields++ > _MAX_YIELDS) {
                break;
            }
        }

        w.vtm = rdvtm(w.rdgen);
    } while (!_done);
}

void Executor::_exploit_task(Worker& w, Node*& t) {
    if (t) {
        const auto d = w.domain;
        if (_num_actives[d].fetch_add(1) == 0 && _num_thieves[d] == 0) {
            _notifier[d].notify(false);
        }

        while (t) {
            OUTPUT_EXECUTOR_STATS("exploit_task");

            _invoke(w, t);
            if (t->_parent == nullptr) {  // detached
                if (t->_topology->_join_counter.fetch_sub(1) == 1) {
                    _tear_down_topology(t->_topology);
                }
            } else {  // joined subflow
                t->_parent->_join_counter.fetch_sub(1);
            }
            t = w.wsq[d].pop();
        }

        --_num_actives[d];
    }
}

bool Executor::_wait_for_task(Worker& worker, Node*& t) {
    const auto d = worker.domain;

wait_for_task:

    assert(!t);
    ++_num_thieves[d];

explore_task:

    _explore_task(worker, t);
    if (t) {
        if (_num_thieves[d].fetch_sub(1) == 1) {
            _notifier[d].notify(false);
        }
        return true;
    }

    _notifier[d].prepare_wait(worker.waiter);
    if (!_wsq[d].empty()) {
        _notifier[d].cancel_wait(worker.waiter);
        t = _wsq[d].steal();  // must steal here
        if (t) {
            if (_num_thieves[d].fetch_sub(1) == 1) {
                _notifier[d].notify(false);
            }
            return true;
        } else {
            worker.vtm = worker.id;
            goto explore_task;
        }
    }

    if (_done) {
        _notifier[d].cancel_wait(worker.waiter);
        for (int i = 0; i < NUM_DOMAINS; ++i) {
            _notifier[i].notify(true);
        }
        --_num_thieves[d];
        return false;
    }

    if (_num_thieves[d].fetch_sub(1) == 1) {
        if (_num_actives[d]) {
            _notifier[d].cancel_wait(worker.waiter);
            goto wait_for_task;
        }
        // check all domain queue again
        for (auto& w : _workers) {
            if (!w.wsq[d].empty()) {
                worker.vtm = w.id;
                _notifier[d].cancel_wait(worker.waiter);
                goto wait_for_task;
            }
        }
    }

    // Now I really need to relinguish my self to others
    _notifier[d].commit_wait(worker.waiter);

    return true;
}

size_t Executor::num_observers() const {
    return _observers.size();
}

// The main procedure to schedule a give task node.
// Each task node has two types of tasks - regular and subflow.
void Executor::_schedule(Node* node) {
    // assert(_workers.size() != 0);

    OUTPUT_EXECUTOR_STATS("schedule");

    const auto d = node->domain();

    // caller is a worker to this pool
    auto worker = _per_thread().worker;

    if (worker != nullptr && worker->executor == this) {
        worker->wsq[d].push(node);
        if (worker->domain != d) {
            if (_num_actives[d] == 0 && _num_thieves[d] == 0) {
                _notifier[d].notify(false);
            }
        }
        return;
    }

    // other threads
    {
        std::lock_guard<std::mutex> lock(_wsq_mutex);
        _wsq[d].push(node);
    }

    _notifier[d].notify(false);
}

// The main procedure to schedule a set of task nodes.
// Each task node has two types of tasks - regular and subflow.
void Executor::_schedule(PassiveVector<Node*>& nodes) {
    // assert(_workers.size() != 0);

    // We need to cacth the node count to avoid accessing the nodes
    // vector while the parent topology is removed!
    const auto num_nodes = nodes.size();

    if (num_nodes == 0) {
        return;
    }

    // worker thread
    auto worker = _per_thread().worker;

    // task counts
    size_t tcount[NUM_DOMAINS] = {0};

    if (worker != nullptr && worker->executor == this) {
        for (size_t i = 0; i < num_nodes; ++i) {
            const auto d = nodes[i]->domain();
            worker->wsq[d].push(nodes[i]);
            tcount[d]++;
        }

        for (int d = 0; d < NUM_DOMAINS; ++d) {
            if (tcount[d] && d != worker->domain) {
                if (_num_actives[d] == 0 && _num_thieves[d] == 0) {
                    _notifier[d].notify_n(tcount[d]);
                }
            }
        }

        return;
    }

    // other threads
    {
        std::lock_guard<std::mutex> lock(_wsq_mutex);
        for (size_t k = 0; k < num_nodes; ++k) {
            const auto d = nodes[k]->domain();
            _wsq[d].push(nodes[k]);
            tcount[d]++;
        }
    }

    for (int d = 0; d < NUM_DOMAINS; ++d) {
        _notifier[d].notify_n(tcount[d]);
    }
}

void Executor::_invoke(Worker& worker, Node* node) {
    // assert(_workers.size() != 0);

    // Here we need to fetch the num_successors first to avoid the invalid
    // memory access caused by topology clear.
    const auto num_successors = node->num_successors();

    // acquire the parent flow counter
    auto& c = (node->_parent) ? node->_parent->_join_counter : node->_topology->_join_counter;

    VLOG(3) << "executor _invoke step 1, node:" << node->dump();

    // switch is faster than nested if-else due to jump table
    switch (node->_handle.index()) {
        // static task
        case Node::STATIC_WORK: {
            _invoke_static_work(worker, node);
        } break;

        // module task
        case Node::MODULE_WORK: {
            _invoke_module_work(worker, node);
        } break;

        // dynamic task
        case Node::DYNAMIC_WORK: {
            _invoke_dynamic_work(worker, node);
        } break;

        // condition task
        case Node::CONDITION_WORK: {
            _invoke_condition_work(worker, node);
            return;
        }  // no need to add a break here due to the immediate return

        // exit task
        case Node::EXIT_WORK: {
            _invoke_exit_work(worker, node);
            return;
        }  // no need to add a break here due to the immediate return

        // monostate
        default:
            break;
    }

    // We MUST recover the dependency since subflow may have
    // a condition node to go back (cyclic).
    // This must be done before scheduling the successors, otherwise this might
    // cause race condition on the _dependents
    if (node->_has_state(Node::BRANCHED)) {
        // If this is a case node, we need to deduct condition predecessors
        node->_join_counter = node->num_strong_dependents();
    } else {
        node->_join_counter = node->num_dependents();
    }

    VLOG(4) << "executor _invoke step 2, node:" << node->dump();

    // At this point, the node storage might be destructed.
    for (size_t i = 0; i < num_successors; ++i) {
        if (--(node->_successors[i]->_join_counter) == 0) {
            c.fetch_add(1);
            VLOG(4) << "executor _invoke step 3, node:" << node->dump();
            _schedule(node->_successors[i]);
        }
    }

    VLOG(3) << "executor _invoke step 4, node:" << node->dump();
}

void Executor::_observer_prologue(Worker& worker, Node* node) {
    for (auto& observer : _observers) {
        observer->on_entry(worker.id, TaskView(node));
    }
}

void Executor::_observer_epilogue(Worker& worker, Node* node) {
    for (auto& observer : _observers) {
        observer->on_exit(worker.id, TaskView(node));
    }
}

void Executor::_invoke_static_work(Worker& worker, Node* node) {
    _observer_prologue(worker, node);
    if (node->_topology->_exit_invoked) {
        VLOG(3) << "topology has been marked as `exit invoked`, so ignore static node:" << node->name();
        node->reset_exec_state(Node::ExecState::IGNORE);
    } else {
        nstd::get<Node::StaticWork>(node->_handle).work();
        node->reset_exec_state(Node::ExecState::DONE);
    }
    _observer_epilogue(worker, node);
}

void Executor::_invoke_dynamic_work(Worker& w, Node* node) {
    _observer_prologue(w, node);

    auto& handle = nstd::get<Node::DynamicWork>(node->_handle);
    handle.subgraph.clear();
    Subflow sf(*this, node, handle.subgraph);
    handle.work(sf);
    node->reset_exec_state(Node::ExecState::DONE);
    if (sf._joinable) {
        _invoke_dynamic_work_internal(w, node, handle.subgraph, false);
    }

    // TODO
    _observer_epilogue(w, node);
}

void Executor::_invoke_dynamic_work_external(Node* p, Graph& g, bool detach) {
    auto worker = _per_thread().worker;
    assert(worker && worker->executor == this);
    _invoke_dynamic_work_internal(*worker, p, g, detach);
}

void Executor::_invoke_dynamic_work_internal(Worker& w, Node* p, Graph& g, bool detach) {
    assert(p);
    if (g.empty()) {
        return;
    }

    PassiveVector<Node*> src;
    for (auto n : g._nodes) {
        n->_topology = p->_topology;
        n->_set_up_join_counter();

        if (detach) {
            n->_parent = nullptr;
            n->_set_state(Node::DETACHED);
        } else {
            n->_parent = p;
        }

        if (n->num_dependents() == 0) {
            src.push_back(n);
        }
    }

    if (detach) {  // detach here
        {
            std::lock_guard<std::mutex> lock(p->_topology->_taskflow._mtx);
            p->_topology->_taskflow._graph.merge(std::move(g));
        }

        p->_topology->_join_counter.fetch_add(src.size());
        _schedule(src);
    } else {  // join here
        p->_join_counter.fetch_add(src.size());
        _schedule(src);
        Node* t = nullptr;

        while (p->_join_counter != 0) {
            t = w.wsq[w.domain].pop();

            if (t) {
                _invoke(w, t);
                t->_parent ? t->_parent->_join_counter.fetch_sub(1) : t->_topology->_join_counter.fetch_sub(1);
            } else {  // here we don't steal (unknown issues to fix)
                std::this_thread::yield();
            }
        }
    }
}

void Executor::_invoke_condition_work(Worker& worker, Node* node) {
    _observer_prologue(worker, node);

    if (node->_has_state(Node::BRANCHED)) {
        node->_join_counter = node->num_strong_dependents();
    } else {
        node->_join_counter = node->num_dependents();
    }

    int id = 0;  // FIXME: default is 0 for exit mode?
    if (node->_topology->_exit_invoked) {
        VLOG(3) << "topology has been marked as `exit invoked`, so ignore condition node:" << node->name();
        node->reset_exec_state(Node::ExecState::IGNORE);
    } else {
        id = nstd::get<Node::ConditionWork>(node->_handle).work();
        node->reset_exec_state(Node::ExecState::DONE);
    }

    if (id >= 0 && static_cast<size_t>(id) < node->num_successors()) {
        auto s = node->_successors[id];
        s->_join_counter.store(0);

        // FIXME:? order before execution of s. not node->_join_counter
        node->_parent ? node->_parent->_join_counter.fetch_add(1) : node->_topology->_join_counter.fetch_add(1);
        _schedule(s);
    }

    _observer_epilogue(worker, node);
}

void Executor::_invoke_exit_work(Worker& worker, Node* node) {
    _observer_prologue(worker, node);

    const auto num_successors = node->num_successors();
    auto& c = (node->_parent) ? node->_parent->_join_counter : node->_topology->_join_counter;

    bool result = true;  // FIXME: default is true for pre exit mode?
    if (node->_topology->_exit_invoked) {
        VLOG(3) << "topology has been marked as `exit invoked`, so ignore exit node:" << node->name();
        node->reset_exec_state(Node::ExecState::IGNORE);
    } else {
        result = nstd::get<Node::ExitWork>(node->_handle).work();
        node->reset_exec_state(Node::ExecState::DONE);
    }

    VLOG(3) << "exit node invoked, then to " << (result ? "continue" : "exit") << " node:" << node->dump();

    if (!result) {
        node->_topology->reset_exit_invoked(true);
        node->reset_exec_state(Node::ExecState::EXIT);
    }

    if (node->_has_state(Node::BRANCHED)) {
        node->_join_counter = node->num_strong_dependents();
    } else {
        node->_join_counter = node->num_dependents();
    }

    for (size_t i = 0; i < num_successors; ++i) {
        if (--(node->_successors[i]->_join_counter) == 0) {
            c.fetch_add(1);
            _schedule(node->_successors[i]);
        }
    }

    _observer_epilogue(worker, node);
}

void Executor::_invoke_module_work(Worker& w, Node* node) {
    _observer_prologue(w, node);
    auto module = nstd::get<Node::ModuleWork>(node->_handle).module;
    _invoke_dynamic_work_internal(w, node, module->_graph, false);
    _observer_epilogue(w, node);
}

std::future<int> Executor::run(Taskflow& f) {
    return run_n(f, 1, []() {});
}

std::future<int> Executor::run_n(Taskflow& f, size_t repeat) {
    return run_n(f, repeat, []() {});
}

int Executor::_set_up_topology(Topology* tpg) {
    tpg->_sources.clear();
    tpg->_taskflow._graph.clear_detached();

    for (auto node : tpg->_taskflow._graph._nodes) {
        node->_topology = tpg;
        node->_clear_state();

        if (node->num_dependents() == 0) {
            tpg->_sources.push_back(node);
        }

        node->_set_up_join_counter();
    }

    tpg->_join_counter.store(tpg->_sources.size(), std::memory_order_relaxed);

    return tpg->_sources.size();
}

void Executor::_tear_down_topology(Topology* tpg) {
    auto& f = tpg->_taskflow;

    // assert(&tpg == &(f._topologies.front()));

    // case 1: we still need to run the topology again
    if (!tpg->_pred()) {
        assert(tpg->_join_counter == 0);
        tpg->_join_counter = tpg->_sources.size();
        _schedule(tpg->_sources);
    }
    // case 2: the final run of this topology
    else {
        if (tpg->_call != nullptr) {
            tpg->_call();
        }

        f._mtx.lock();

        // If there is another run (interleave between lock)
        if (f._topologies.size() > 1) {
            assert(tpg->_join_counter == 0);

            // Set the promise
            tpg->_promise.set_value(0);
            f._topologies.pop_front();
            f._mtx.unlock();

            // decrement the topology but since this is not the last we don't
            // notify
            _decrement_topology();

            tpg = &(f._topologies.front());

            _set_up_topology(tpg);
            _schedule(tpg->_sources);
        } else {
            assert(f._topologies.size() == 1);

            // Need to back up the promise first here becuz taskflow might be
            // destroy before taskflow leaves
            std::promise<int> p{std::move(tpg->_promise)};

            // Back up lambda capture in case it has the topology pointer, to
            // avoid it releasing on
            // pop_front ahead of _mtx.unlock & _promise.set_value. Released
            // safely when leaving scope.
            auto bc{std::move(tpg->_call)};

            f._topologies.pop_front();

            f._mtx.unlock();

            // We set the promise in the end in case taskflow leaves before
            // taskflow
            p.set_value(0);

            _decrement_topology_and_notify();
        }
    }
}

void Executor::_increment_topology() {
    std::lock_guard<std::mutex> lock(_topology_mutex);
    ++_num_topologies;
}

void Executor::_decrement_topology_and_notify() {
    std::lock_guard<std::mutex> lock(_topology_mutex);
    if (--_num_topologies == 0) {
        _topology_cv.notify_all();
    }
}

void Executor::_decrement_topology() {
    std::lock_guard<std::mutex> lock(_topology_mutex);
    --_num_topologies;
}

void Executor::wait_for_all() {
    std::unique_lock<std::mutex> lock(_topology_mutex);
    _topology_cv.wait(lock, [&]() { return _num_topologies == 0; });
}

}  // end of namespace knitter