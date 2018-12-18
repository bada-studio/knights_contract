#pragma once

class dungeon_random {
public:
    uint32_t seed;

    int range(uint32_t to) {
        const uint64_t a = 1103515245;
        const uint64_t c = 12345;
        seed = (uint32_t)((a * seed + c) % 0x7fffffff);
        int value = (int)(seed % to);
        return value;
    }
};