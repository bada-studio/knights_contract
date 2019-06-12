enum skin_state {
    ss_normal = 0,
    ss_wear,
    ss_selling,
};

// 16 bytes
struct skinrow {
    uint32_t cid = 0;
    uint16_t code = 0;
    uint8_t state = 0; // 0: normal, 1: wear, 2: selling
};

struct [[eosio::table]] skin {
    name owner;
    std::vector<skinrow> rows;

    uint64_t primary_key() const {
        return owner.value;
    }

    int get_skin(uint32_t cid) const {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].cid == cid) { 
                return index;
            }
        }

        return -1;
    }

    int get_skin_by_code(uint16_t code) const {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].code == code) { 
                return index;
            }
        }

        return -1;
    }

    EOSLIB_SERIALIZE(
            skin,
            (owner)
            (rows)
    )
};

typedef eosio::multi_index< "skin"_n, skin > skin_table;
