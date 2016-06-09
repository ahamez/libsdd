/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_taskmodelgenerator__
#define __scheduling_taskmodelgenerator__

#include <algorithm>
#include <cmath>
#include <random>
#include <set>

#include "TaskModel.hpp"


TaskModel generate_task_model(uint32_t task_number) {
    // Build random generators.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dep_dist(task_number / 2, task_number);
    std::uniform_int_distribution<> task_dist(0, task_number - 1);
    std::uniform_int_distribution<> wcet_dist(5, 10);

    TaskModel rv(task_number);

    uint32_t max_bound = 0;

    for (uint32_t i = 0; i < task_number; ++i) {
        // Generate random dependencies.
        uint32_t dependency_number = std::min<uint32_t>(dep_dist(gen), i);
        std::set<uint32_t> deps;

        while (deps.size() < dependency_number) {
            uint32_t j = task_dist(gen) % i;
            rv.add_dependency(i, j);
            deps.insert(j);
        }

        // Compute the worst-case execution time.
        uint32_t wcet = wcet_dist(gen);

        // Compute the task release time.
        uint32_t hard_release = 0;
        for (auto j : deps) {
            if (rv.task(j).deadline > hard_release) {
                hard_release = rv.task(j).deadline;
            }
        }
        uint32_t release = std::max(hard_release, 2 * wcet) - 2 * wcet;

        // Compute the dealdine time.
        uint32_t deadline = (release + 2.5 * wcet);

        // Add the task to the task model.
        rv.task(i) = std::make_tuple(release, wcet, deadline, 0);

        max_bound = deadline;
    }

    return rv;
}

#endif
