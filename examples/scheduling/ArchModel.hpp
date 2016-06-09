// Copyright (c) 2016, Dimitri Racordon.
// Licensed under the BSD License.

#ifndef __scheduling_archmodel__
#define __scheduling_archmodel__

#include <tuple>
#include <vector>


class ArchModel {
private:
    struct Core;

public:
    ArchModel(uint32_t core_number)
    : core_number(core_number), _cores(core_number) {
    }

    Core& core(uint32_t t) {
        return this->_cores[t];
    }

    const uint32_t core_number;

private:
    struct Core {
        Core()
        : scaling_factor(1.0) {
        }

        Core& operator=(const std::tuple<float>& t) {
            this->scaling_factor = std::get<0>(t);
            return *this;
        }

        float scaling_factor;
    };

    std::vector<Core> _cores;
};

#endif
