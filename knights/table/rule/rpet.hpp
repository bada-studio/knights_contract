enum pet_grade {
    pg_none = 0,
    pg_normal, // 1
    pg_rare, // 2
    pg_unique, // 3
    pg_legendary, // 4
    pg_ancient, // 5
    pg_count
};

enum pet_gacha_type {
    pgt_none = 0,
    pgt_low_class, // 1
    pgt_high_class, // 2
    pgt_count,
};

//@abi table rpet i64
struct rpet {
    uint64_t code = 0;
    uint8_t grade = 0;
    uint8_t stat1_type = 0;
    uint8_t stat2_type = 0;
    uint8_t stat3_type = 0;
    uint16_t stat1 = 0;
    uint16_t stat2 = 0;
    uint16_t stat3 = 0;
    uint8_t stat1_up_per_level = 0;
    uint8_t stat2_up_per_level = 0;
    uint8_t stat3_up_per_level = 0;
    uint32_t relative_drop_rate = 0;

    rpet() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rpet,
            (code)
            (grade)
            (stat1_type)
            (stat2_type)
            (stat3_type)
            (stat1)
            (stat2)
            (stat3)
            (stat1_up_per_level)
            (stat2_up_per_level)
            (stat3_up_per_level)
            (relative_drop_rate)
    )
};

typedef eosio::multi_index< N(rpet), rpet> rpet_table;
