/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_filters__
#define __scheduling_filters__

#include "sdd/sdd.hh"

#include "Config.hpp"
#include "TaskModel.hpp"


using Identifier = conf::Identifier;
using Values = conf::Values;
using ValuesBuilder = sdd::values::values_traits<Values>::builder;


/// Filter that filters out the paths according to the presence (or not) of
/// a given constant arc value.
struct filter_constant {
    /// The constant value to check.
    const Arc constant;
    /// Whether or not keep the arcs whose value is equal to the constant.
    const bool keep;

    Values operator()(const Values& val) const {
        auto builder = ValuesBuilder();
        builder.reserve(val.size());

        // Keeps the arcs whose value is equal to (resp. different than) the
        // given constant if keep is true (resp. false).
        std::copy_if(
            val.cbegin(),
            val.cend(),
            std::inserter(builder, builder.end()),
            [this](auto x){
                return (x == this->constant) == this->keep;
            }
        );

        return std::move(builder);
    }

    bool selector() const noexcept {
        return true;
    }

    friend bool operator==(const filter_constant& lhs, const filter_constant& rhs) noexcept {
        return (lhs.constant == rhs.constant) and (lhs.keep == rhs.keep);
    }
};


/// Filter that filters out the paths that represent a situtation where the
/// given task cannot be scheduled on the given core without violating its
/// timing constraints.
struct filter_constraints {
    const Identifier task;
    const TaskModel& task_model;

    Values operator()(const Values& val) const {
        auto builder = ValuesBuilder();
        builder.reserve(val.size());

        // Filters out arcs for which the ETA of the task would be greater
        // than its deadline time.
        std::copy_if(
            val.cbegin(),
            val.cend(),
            std::inserter(builder, builder.end()),
            [this](auto x){
                uint32_t deadline = this->task_model.task(this->task).deadline;
                return this->task_model.eta(this->task, x.time) <= deadline;
            }
        );

        return std::move(builder);
    }

    bool selector() const noexcept {
        return true;
    }

    friend bool operator==(const filter_constraints& lhs, const filter_constraints& rhs) noexcept {
        return (lhs.task == rhs.task) and (&lhs.task_model == &rhs.task_model);
    }
};


namespace std {

    template<>
    struct hash<filter_constant> {
        std::size_t operator()(const filter_constant& f) const noexcept {
            using namespace sdd::hash;
            return seed(3464152273) (val(f.constant)) (val(f.keep));
        }
    };

    template<>
    struct hash<filter_constraints> {
        std::size_t operator()(const filter_constraints& f) const noexcept {
            using namespace sdd::hash;
            return seed(3464152273) (val(f.task)) (val(&f.task_model));
        }
    };

}


#endif
