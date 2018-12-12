#pragma once

struct rmobskill {
    uint16_t code;
    uint8_t type;
    uint8_t scope;
    uint8_t target;

    uint8_t stat1type;
    uint8_t stat1target;
    uint16_t stat1;
    
    uint8_t stat2type;
    uint8_t stat2target;
    uint16_t stat2;

    uint8_t stat3type;
    uint8_t stat3target;
    uint16_t stat3;
};

//@abi table rmobskills i64
struct rmobskills {
    uint64_t no = 0;
    std::vector<rmobskill> skills;

    rmobskills() {
    }

    uint64_t primary_key() const {
        return no;
    }

    const rmobskill& get_rule(int id) const {
        for (int index = 0; index < skills.size(); index++) {
            if (skills[index].code == id) {
                return skills[index];
            }
        }

        eosio_assert(0, "can not found skill rule");
        return skills[0];
    }

    EOSLIB_SERIALIZE(
            rmobskills,
            (no)
            (skills)
    )
};

typedef eosio::multi_index< N(rmobskills), rmobskills> rmobskills_table;
