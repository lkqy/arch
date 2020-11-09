#pragma once
#include "knitter/common/err_code.h"
#include "knitter/task/notifier.h"
#include "knitter/task/observer.h"
#include "knitter/task/subflow.h"
#include "knitter/task/taskflow.h"
#include "knitter/task/tsq.h"

namespace knitter {

class Executor {
    friend class Subflow;

    struct Worker {
        size_t id;
        size_t vtm;
        Domain domain;
        Executor* executor;
        Notifier::Waiter* waiter;
        std::mt19937 rdgen{std::random_device{}()};
        TaskQueue<Node*> wsq[NUM_DOMAINS];
    };

    struct PerThread {
        Worker* worker{nullptr};
    };

public:
    explicit Executor(size_t N = std::thread::hardware_concurrency(), const std::string& thread_name = "knitter_dag");

    ~Executor();

    std::future<int> run(Taskflow& taskflow);

    template <typename C>
    std::future<int> run(Taskflow& taskflow, C&& callable);

    std::future<int> run_n(Taskflow& taskflow, size_t N);

    template <typename C>
    std::future<int> run_n(Taskflow& taskflow, size_t N, C&& callable);

    template <typename P>
    std::future<int> run_until(Taskflow& taskflow, P&& pred);

    template <typename P, typename C>
    std::future<int> run_until(Taskflow& taskflow, P&& pred, C&& callable);

    void wait_for_all();

    // queries the number of worker threads (can be zero)
    size_t num_workers() const;

    // queries the number of running topologies at the time of this call
    // When a taskflow is submitted to an executor, a topology is created to
    // store
    // runtime metadata of the running taskflow.
    size_t num_topologies() const;

    // queries the number of worker domains
    // Each domain manages a subset of worker threads to execute domain-specific
    // tasks, for example, HOST tasks.
    size_t num_domains() const;

    // queries the id of the caller thread in this executor
    // Each worker has an unique id from 0 to N-1 exclusive to the associated
    // executor. If the caller thread does not belong to the executor, -1 is
    // returned.
    int this_worker_id() const;

    template <typename Observer, typename... Args>
    std::shared_ptr<Observer> make_observer(Args&&... args);

    template <typename Observer>
    void remove_observer(std::shared_ptr<Observer> observer);

    size_t num_observers() const;

private:
    const size_t _VICTIM_BEG;
    const size_t _VICTIM_END;
    const size_t _MAX_STEALS;
    const size_t _MAX_YIELDS;

    std::condition_variable _topology_cv;
    std::mutex _topology_mutex;
    std::mutex _wsq_mutex;

    size_t _num_topologies{0};

    std::vector<Worker> _workers;
    std::vector<std::thread> _threads;

    Notifier _notifier[NUM_DOMAINS];

    TaskQueue<Node*> _wsq[NUM_DOMAINS];

    std::atomic<size_t> _num_actives[NUM_DOMAINS];
    std::atomic<size_t> _num_thieves[NUM_DOMAINS];
    std::atomic<bool> _done{0};

    std::unordered_set<std::shared_ptr<ObserverInterface>> _observers;

    TFProfObserver* _tfprof;

    PerThread& _per_thread() const;

    bool _wait_for_task(Worker&, Node*&);

    void _instantiate_tfprof();
    void _flush_tfprof();
    void _observer_prologue(Worker&, Node*);
    void _observer_epilogue(Worker&, Node*);
    void _spawn(size_t, Domain, const std::string& thread_name);
    void _worker_loop(Worker&);
    void _exploit_task(Worker&, Node*&);
    void _explore_task(Worker&, Node*&);
    void _schedule(Node*);
    void _schedule(PassiveVector<Node*>&);
    void _invoke(Worker&, Node*);
    void _invoke_static_work(Worker&, Node*);
    void _invoke_dynamic_work(Worker&, Node*);
    void _invoke_dynamic_work_internal(Worker&, Node*, Graph&, bool);
    void _invoke_dynamic_work_external(Node*, Graph&, bool);
    void _invoke_condition_work(Worker&, Node*);
    void _invoke_module_work(Worker&, Node*);
    void _invoke_exit_work(Worker&, Node*);

    int _set_up_topology(Topology*);
    void _tear_down_topology(Topology*);
    void _increment_topology();
    void _decrement_topology();
    void _decrement_topology_and_notify();
};

template <typename Observer, typename... Args>
std::shared_ptr<Observer> Executor::make_observer(Args&&... args) {
    static_assert(std::is_base_of<ObserverInterface, Observer>::value,
                  "Observer must be derived from ObserverInterface");
    auto ptr = std::make_shared<Observer>(std::forward<Args>(args)...);
    ptr->set_up(_workers.size());
    _observers.emplace(std::static_pointer_cast<ObserverInterface>(ptr));
    return ptr;
}

template <typename Observer>
void Executor::remove_observer(std::shared_ptr<Observer> ptr) {
    static_assert(std::is_base_of<ObserverInterface, Observer>::value,
                  "Observer must be derived from ObserverInterface");
    _observers.erase(std::static_pointer_cast<ObserverInterface>(ptr));
}

template <typename C>
std::future<int> Executor::run(Taskflow& f, C&& c) {
    return run_n(f, 1, std::forward<C>(c));
}

template <typename C>
std::future<int> Executor::run_n(Taskflow& f, size_t repeat, C&& c) {
    return run_until(f, [repeat]() mutable { return repeat-- == 0; }, std::forward<C>(c));
}

template <typename P>
std::future<int> Executor::run_until(Taskflow& f, P&& pred) {
    return run_until(f, std::forward<P>(pred), []() {});
}

template <typename P, typename C>
std::future<int> Executor::run_until(Taskflow& f, P&& pred, C&& c) {
    _increment_topology();

    // Special case of predicate
    if (f.empty() || pred()) {
        std::promise<int> promise;
        promise.set_value(0);
        _decrement_topology_and_notify();
        return promise.get_future();
    }

    // Multi-threaded execution.
    bool run_now{false};
    Topology* tpg;
    std::future<int> future;

    {
        std::lock_guard<std::mutex> lock(f._mtx);

        // create a topology for this run
        f._topologies.emplace_back(f, std::forward<P>(pred), std::forward<C>(c));
        tpg = &(f._topologies.back());
        future = tpg->_promise.get_future();

        if (f._topologies.size() == 1) {
            run_now = true;
        }
    }

    // Notice here calling schedule may cause the topology to be removed sonner
    // before the function leaves.
    if (run_now) {
        _set_up_topology(tpg);
        _schedule(tpg->_sources);
    }

    return future;
}

}  // end of namespace knitter