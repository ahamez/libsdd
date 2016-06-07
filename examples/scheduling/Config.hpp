/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2016 Dimitri Racordon.
/// @author Dimitri Racordon

#ifndef __scheduling_config__
#define __scheduling_config__

#include "sdd/sdd.hh"


struct Arc {
    uint32_t core;
    uint32_t time;

    Arc(uint32_t core, uint32_t time) : core(core), time(time) {
    }
    
    bool operator==(const Arc& other) const noexcept {
        return (this->core == other.core) and (this->time == other.time);
    }
    
    bool operator<(const Arc& other) const noexcept {
        return (this->core < other.core)
            or ((this->core == other.core) and (this->time < other.time));
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Arc& arc) {
        if (arc.core == Arc::_epsilon) {
            return os << "epsilon";
        } else {
            return os << "(" << arc.core << ", " << arc.time << ")";
        }
    }
    
    /// A special value that represents the absence of core assignement.
    static const uint32_t _epsilon = (uint32_t)(-1);

    static const Arc epsilon() {
        return Arc(Arc::_epsilon, 0);
    }
};


namespace std {
    
    template<>
    struct hash<Arc> {
        std::size_t operator()(const Arc& a) const noexcept {
            using namespace sdd::hash;
            return seed(3464152273) (val(a.core)) (val(a.time));
        }
    };
    
}


struct conf : public sdd::flat_set_default_configuration {
    
    // Implemention note:
    // Because we have to represent two kinds of identifiers (task and cores),
    // within the same numerical type, we will set the most significant bit of
    // the core identifiers to 1, so they don't collide with task identifiers.

    using Identifier = uint32_t;
    using Values = sdd::values::flat_set<Arc>;
};

#endif
