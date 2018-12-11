#pragma once

struct random_val {
    uint32_t seed;
    uint32_t value;

    random_val(uint32_t seed, uint32_t value) {
        this->seed = seed;
        this->value = value;
    }

    uint32_t range(uint32_t to) {
        const uint64_t a = 1103515245;
        const uint64_t c = 12345;

        seed = (uint32_t)((a * seed + c) % 0x7fffffff);
        value = (uint32_t)(seed % to);
        return value;
    }
};
