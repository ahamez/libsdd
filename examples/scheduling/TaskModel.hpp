/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_taskmodel__
#define __scheduling_taskmodel__

#include <tuple>
#include <unordered_set>
#include <vector>


class TaskModel {
private:
    struct Task;

public:
    using TaskSet = std::unordered_set<uint32_t>;

    TaskModel(uint32_t task_number)
    : task_number(task_number), _tasks(task_number), _dependencies(task_number) {
    }

    inline Task& task(uint32_t t) {
        return this->_tasks[t];
    }

    inline const Task& task(uint32_t t) const {
        return this->_tasks[t];
    }

    void add_dependency(uint32_t t, uint32_t u) {
        this->_dependencies[t].insert(u);
    }

    const TaskSet& dependencies_of(uint32_t t) const {
        return this->_dependencies[t];
    }

    /// Return the estimated time to start (ETS) of a task `t`, if scheduled
    /// on a core whose next available time is `tau`.
    uint32_t ets(uint32_t t, uint32_t tau) const {
        return std::max(tau, this->_tasks[t].release);
    }
    
    /// Return the estimated time to arrive (ETA) of a task `t`, if scheduled
    /// on a core whose next available time is `tau`.
    uint32_t eta(uint32_t t, uint32_t tau) const {
        return this->ets(t, tau) + this->_tasks[t].wcet;
    }

    /// Create a task order, according to their dependencies.
    const std::vector<uint32_t> build_order() const {
        std::vector<uint32_t> rv;

        // Build a topological order using Kahn's algorithm.
        TaskSet free_tasks;
        TaskSet dependent_tasks;

        for (uint32_t i = 0; i < this->_dependencies.size(); ++i) {
            if (this->_dependencies[i].empty()) {
                free_tasks.insert(i);
            } else {
                dependent_tasks.insert(i);
            }
        }

        rv.reserve(this->_tasks.size());
        while (!free_tasks.empty()) {
            // Pick a free task and add it to the order.
            auto next = free_tasks.begin();
            free_tasks.erase(next);
            rv.push_back(*next);

            // For all depedent tasks, check if their dependencies are now
            // satisfied and put them in the set of free tasks if they are.
            auto t = dependent_tasks.begin();
            while (t != dependent_tasks.end()) {
                bool is_now_free = true;
                for (auto u : this->dependencies_of(*t)) {
                    if (free_tasks.find(u) != free_tasks.end()) {
                        is_now_free = false;
                        break;
                    }
                }

                if (is_now_free) {
                    free_tasks.insert(*t);
                    t = dependent_tasks.erase(t);
                } else {
                    ++t;
                }
            }
        }

        return rv;
    }

    const uint32_t task_number;

private:
    struct Task {
        Task()
        : release(0), wcet(0), deadline(0), criticality(0) {
        }

        Task& operator=(const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>& t) {
            this->release = std::get<0>(t);
            this->wcet = std::get<1>(t);
            this->deadline = std::get<2>(t);
            this->criticality = std::get<3>(t);

            return *this;
        }

        uint32_t release;
        uint32_t wcet;
        uint32_t deadline;
        uint32_t criticality;
    };

    std::vector<Task> _tasks;
    std::vector<TaskSet> _dependencies;
};

#endif
