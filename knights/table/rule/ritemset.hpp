struct ritemsetrow {
    uint8_t type;
    uint16_t stat;
};

//@abi table ritemset i64
struct ritemset {
    uint64_t setid;

    // element1
    uint16_t e1_code;
    uint8_t e1_type;
    uint16_t e1_stat;

    // element2
    uint16_t e2_code;
    uint8_t e2_type;
    uint16_t e2_stat;

    // element3
    uint16_t e3_code;
    uint8_t e3_type;
    uint16_t e3_stat;

    // element4
    uint16_t e4_code;
    uint8_t e4_type;
    uint16_t e4_stat;

    // element5
    uint16_t e5_code;
    uint8_t e5_type;
    uint16_t e5_stat;

    uint64_t primary_key() const {
        return setid;
    }

    ritemsetrow get_element(uint16_t code) const {
        ritemsetrow result;
        if (code == e1_code) {
            result.type = e1_type;
            result.stat = e1_stat;
        } else if (code == e2_code) {
            result.type = e2_type;
            result.stat = e2_stat;
        } else if (code == e3_code) {
            result.type = e3_type;
            result.stat = e3_stat;
        } else if (code == e4_code) {
            result.type = e4_type;
            result.stat = e4_stat;
        } else if (code == e5_code) {
            result.type = e5_type;
            result.stat = e5_stat;
        }

        return result;
    }

    EOSLIB_SERIALIZE(
            ritemset,
            (setid)
            (e1_code)
            (e1_type)
            (e1_stat)

            (e2_code)
            (e2_type)
            (e2_stat)

            (e3_code)
            (e3_type)
            (e3_stat)

            (e4_code)
            (e4_type)
            (e4_stat)

            (e5_code)
            (e5_type)
            (e5_stat)
    )
};

typedef eosio::multi_index< N(ritemset), ritemset > ritemset_table;
