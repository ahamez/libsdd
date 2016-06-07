/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_nodecounter__
#define __scheduling_nodecounter__

#include <sstream>
#include <unordered_set>

#include "sdd/dd/definition.hh"
#include "sdd/order/order.hh"


namespace sdd {
namespace tools {


template <typename C>
struct node_counter_visitor{
    std::unordered_set<const void*> _visited;
    std::size_t& counter;
    
    node_counter_visitor(std::size_t& counter)
    : _visited(), counter(counter) {
    }

    // |0|
    void operator()(const sdd::zero_terminal<C>& n, const sdd::order<C>&, unsigned int) {
        const auto search = this->_visited.find(&n);
        if (search == this->_visited.end()) {
            this->_visited.emplace_hint(search, &n);
            this->counter += 1;
        }
    }

    // |1|
    void operator()(const sdd::one_terminal<C>& n, const sdd::order<C>&, unsigned int) {
        const auto search = this->_visited.find(&n);
        if (search == this->_visited.end()) {
            this->_visited.emplace_hint(search, &n);
            this->counter += 1;
        }
    }

    //Flat SDD.
    void operator()(const flat_node<C>& n, const order<C>& o, unsigned int depth) {
        const auto search = this->_visited.find(&n);
        if (search == this->_visited.end()) {
            this->_visited.emplace_hint(search, &n);
            this->counter += 1;

            for (const auto& arc : n) {
                visit(*this, arc.successor(), o.next(), depth);
            }
        }
    }

    /// Hierarchical SDD.
    void operator()(const hierarchical_node<C>& n, const order<C>&, unsigned int depth) {
    }
};


template <typename C>
struct node_counter {
    const SDD<C> x_;
    const order<C>& o_;

    friend std::ostream& operator<<(std::ostream& out, const node_counter& manip) {
        std::size_t counter = 0;
        visit(node_counter_visitor<C>(counter), manip.x_, manip.o_, 0);
        return out << counter;
    }
};


template <typename C>
node_counter<C> node_count(const SDD<C>& x, const order<C>& o) {
    return {x, o};
}
    
} // namespace tools
} // namespace sdd

#endif
