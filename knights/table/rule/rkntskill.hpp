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

    EOSLIB_SERIALIZE(
            rkntskills,
            (no)
            (skills)
    )
};

typedef eosio::multi_index< N(rkntskills), rkntskills> rkntskills_table;
