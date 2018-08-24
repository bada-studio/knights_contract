#pragma once

struct random_val {
    uint64_t seed;
    uint32_t value;

    random_val(uint64_t seed, uint32_t value) {
        this->seed = seed;
        this->value = value;
    }
};
