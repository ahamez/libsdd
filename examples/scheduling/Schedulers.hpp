/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_schedulers__
#define __scheduling_schedulers__

#include "sdd/sdd.hh"

#include "Config.hpp"
#include "TaskModel.hpp"


using Hom = sdd::homomorphism<conf>;
using SDD = sdd::SDD<conf>;

using Identifier = conf::Identifier;
using Values = conf::Values;
using ValuesBuilder = sdd::values::values_traits<Values>::builder;



/// Schedule a given task, on a given core, at a gien time.
struct schedule_tau {
    const Identifier task;
    const Identifier core;
    const uint32_t tau;

    Values operator()(const Values& val) const {
        auto builder = ValuesBuilder();
        builder.reserve(val.size());

        // Update the arc value of the given task to set (core, tau).
        for (std::size_t i = 0; i < val.size(); ++i) {
            builder.insert(Arc(core, tau));
        }

        return std::move(builder);
    }
    
    bool selector() const noexcept {
        return true;
    }
    
    friend bool operator==(const schedule_tau& lhs, const schedule_tau& rhs) noexcept {
        return (lhs.task == rhs.task) and (lhs.core == rhs.core)
        and (lhs.tau == rhs.tau);
    }
};


/// Update the clock time of a core to the next time it will be available
/// scheduling the given task, and schedule the given task on it.
struct schedule {
    const Identifier task;
    const Identifier core;
    const TaskModel& task_model;

    bool skip(const Identifier var) const noexcept {
        // notice the bit mask!
        return var != (this->core | (1 << 31));
    }

    Hom operator()(const sdd::order<conf>&, const SDD&) const noexcept {
        // no hierarchy
        assert(false);
        __builtin_unreachable();
    }

    Hom operator()(const sdd::order<conf>& o, const Values& val) const {
        Arc arc = *val.begin();

        uint32_t tau = arc.time;
        uint32_t ets = this->task_model.ets(this->task, tau);
        arc.time = this->task_model.eta(this->task, arc.time);

        // Create the new mapping for the current core variable, so that the
        // the outgoing arc is labeled with the next available time of the
        // given core, and the child DD schedules the given task at its
        // estimated time to start.
        return sdd::cons<conf>(
            o,
            std::move(Values{arc}),
            sdd::function(o, this->task, schedule_tau{this->task, this->core, ets}));
    }

    SDD operator()() const noexcept {
        return sdd::zero<conf>();
    }

    friend bool operator==(const schedule& lhs, const schedule& rhs) noexcept {
        return (lhs.task == rhs.task) and (lhs.core == rhs.core)
            and (&lhs.task_model == &rhs.task_model);
    }
};


namespace std {

    template<>
    struct hash<schedule_tau> {
        std::size_t operator()(const schedule_tau& s) const noexcept {
            using namespace sdd::hash;
            return seed(3464152273) (val(s.task)) (val(s.core)) (val(s.tau));
        }
    };

    template<>
    struct hash<schedule> {
        std::size_t operator()(const schedule& f) const noexcept {
            using namespace sdd::hash;
            return seed(3464152273) (val(f.task)) (val(f.core)) (val(&f.task_model));
        }
    };
    
}

#endif
