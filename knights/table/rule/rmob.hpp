#pragma once

struct rmob {
    uint16_t name = 0;
    uint16_t rate = 0;
    uint8_t type = 0;
    uint16_t attack = 0;
    uint16_t defense = 0;
    uint16_t hp = 0;
    uint16_t skill1 = 0;
    uint16_t skill2 = 0;
    uint16_t skill3 = 0;
    uint16_t skill4 = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
};

//@abi table rmobs i64
struct rmobs {
    uint64_t code = 0;
    std::vector<rmob> mob;
    std::vector<rmob> mboss;
    std::vector<rmob> boss;

    rmobs() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rmobs,
            (code)
            (mob)
            (mboss)
            (boss)
    )
};

typedef eosio::multi_index< N(rmobs), rmobs> rmobs_table;
