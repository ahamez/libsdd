/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#include <chrono>
#include <iostream>
#include <boost/range/adaptor/reversed.hpp>
#include <set>

#include "sdd/sdd.hh"
#include "sdd/tools/dot/sdd.hh"
#include "sdd/tools/nodes.hh"

#include "ArchModel.hpp"
#include "Config.hpp"
#include "Filters.hpp"
#include "Schedulers.hpp"
#include "TaskModel.hpp"
#include "TaskModelGenerator.hpp"


using SDD = sdd::SDD<conf>;
using Hom = sdd::homomorphism<conf>;
using Identifier = conf::Identifier;
using Values = conf::Values;


int main(int argc, const char * argv[]) {
    // Initialize sddlib.
    auto manager = sdd::init<conf>();

    // Create the architectur model.
    uint32_t core_number = 1;
    if (argc >= 3) {
        core_number = atoi(argv[2]);
    }
    ArchModel am(core_number);

    // Create a task model.
    uint32_t task_number = 3;
    if (argc >= 2) {
        task_number = atoi(argv[1]);
    }
    TaskModel tm = generate_task_model(task_number);

    // Create the variable order with the core variables first, followed by
    // the task variables.
    sdd::order_builder<conf> ob;

    const auto task_order = tm.build_order();
    for (uint32_t i = (uint32_t)task_order.size(); i > 0; --i) {
        ob.push(task_order[i - 1]);
    }

    for (uint32_t i = 0; i < am.core_number; ++i) {
        ob.push(i | (1 << 31));
    }
    sdd::order<conf> order(ob);

    // Create the initial state.
    SDD m0(order, [](Identifier e){
        if (e & (1 << 31)) {
            // Assign core variables to (0, 0).
            return Values{Arc(0, 0)};
        } else {
            // Assign task variables to (epsilon, 0).
            return Values{Arc::epsilon()};
        }
    });

    auto all_schedulers_set = std::set<Hom>{};

    // Compute all the schedulers for t.
    for (Identifier t = 0; t < tm.task_number; ++t) {
        auto t_schedulers_set = std::set<Hom>{};
        auto filters_set = std::set<Hom>{};

        // We make a filter for the dependencies of t.
        for (auto u : tm.dependencies_of(t)) {
            filters_set.insert(sdd::function<conf>(order, u, filter_constant{Arc::epsilon(), false}));
        }

        // We make a filter for the schedulings where t is already assigned.
        filters_set.insert(sdd::function<conf>(order, t, filter_constant{Arc::epsilon(), true}));

        // The remaining also depends on the core ...
        for (Identifier c = 0; c < am.core_number; ++c) {
            // We make a filter for the timing constraints of t on c.
            filters_set.insert(sdd::function<conf>(order, c | (1 << 31), filter_constraints{t, tm}));

            // We combine all the filters
            const auto filters = sdd::intersection(order, filters_set.begin(), filters_set.end());

            // We make the scheduler for t on c.
            const auto scheduler = sdd::inductive<conf>(schedule{t, c, tm});

            // Finally, the homomorphism for t on c is given by the composition of the
            // filters with the scheduler.
            t_schedulers_set.insert(sdd::composition(scheduler, filters));
        }

        // The homomorphisms for the schedulings of t is given by the union of the
        // schedulers for t on all cores.
        all_schedulers_set.insert(sdd::sum(order, t_schedulers_set.begin(), t_schedulers_set.end()));
    }

    // The final homomorphism is the fixed point on the union of all schedulers for all tasks.
    all_schedulers_set.insert(sdd::id<conf>());
    Hom all_schedulers = sdd::sum(order, all_schedulers_set.begin(), all_schedulers_set.end());
    all_schedulers = sdd::fixpoint(all_schedulers);
    all_schedulers = sdd::rewrite(order, all_schedulers);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::size_t elapsed;

    start = std::chrono::system_clock::now();
    SDD schedulings = all_schedulers(order, m0);
    end = std::chrono::system_clock::now();

    // Filter the result to remove the scheduling that are not feasible for all tasks.
    auto feasible_set = std::set<Hom>{};
    for (Identifier t = 0; t < tm.task_number; ++t) {
        feasible_set.insert(sdd::function<conf>(order, t, filter_constant{Arc::epsilon(), false}));
    }
    Hom feasible = sdd::intersection(order, feasible_set.begin(), feasible_set.end());
    feasible = sdd::rewrite(order, feasible);
    schedulings = feasible(order, schedulings);

    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Time: " << elapsed << "ms" << std::endl;
    std::cout << "Number of nodes: " << sdd::tools::nodes(schedulings).first << std::endl;
    std::cout << "Number of schedulings: " << schedulings.size() << std::endl;

    return 0;
}
