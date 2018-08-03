#pragma once

// Linear Congruential Generator
class random_gen {
private:
    static random_gen instance;

    const uint32_t a = 1103515245;
    const uint32_t c = 12345;
    uint64_t seed = 0;

public:
    static random_gen& get_instance(account_name player) {
        if (instance.seed == 0) {
            instance.seed = tapos_block_prefix() + player;
        }
        return instance;
    }

    uint32_t range(uint32_t to) {
        checksum256 result;
        sha256((char *)&seed, sizeof(seed), &result);
        seed = result.hash[1];
        seed <<= 32;
        seed |= result.hash[0];
        return (uint32_t)(seed % to);

        // old implementation
        // seed = (a * seed + c) % 0x7fffffff;
        // return (uint32_t)(seed % to);
    }
};
