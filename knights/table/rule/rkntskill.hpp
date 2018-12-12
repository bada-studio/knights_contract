#pragma once

struct rkntskill {
    uint16_t code;
    uint8_t knight;
    uint8_t cost;
    uint8_t maxlevel;
    uint8_t requiredlv;
    uint8_t type;
    uint8_t scope;
    uint8_t target;

    uint8_t stat1type;
    uint8_t stat1target;
    uint16_t stat1;
    uint16_t stat1lvbonus;
    
    uint8_t stat2type;
    uint8_t stat2target;
    uint16_t stat2;
    uint16_t stat2lvbonus;

    uint8_t stat3type;
    uint8_t stat3target;
    uint16_t stat3;
    uint16_t stat3lvbonus;
};

//@abi table rkntskills i64
struct rkntskills {
    uint64_t no = 0;
    std::vector<rkntskill> skills;

    rkntskills() {
    }

    uint64_t primary_key() const {
        return no;
    }

    const rkntskill& get_rule(int id) const {
        for (int index = 0; index < skills.size(); index++) {
            if (skills[index].code == id) {
                return skills[index];
            }
        }

        eosio_assert(0, "can not found skill rule");
        return skills[0];
    }

    EOSLIB_SERIALIZE(
            rkntskills,
            (no)
            (skills)
    )
};

typedef eosio::multi_index< N(rkntskills), rkntskills> rkntskills_table;
