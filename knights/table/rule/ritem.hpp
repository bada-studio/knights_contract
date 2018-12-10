enum item_type {
    it_none = 0,
    it_weapon, // 1
    it_armor, // 2
    it_accessory, // 3
    it_count
};

enum item_sub_type {
    ist_none = 0,
    ist_sword = 1,
    ist_axe = 2,
    ist_bow = 3,
    ist_staff = 4,
    ist_leather_armor = 5,
    ist_plate_armor = 6,
    ist_ring = 7,
    ist_amulet = 8,
};

enum stat_type {
    st_none = 0,
    st_attack, // 1
    st_defense, // 2
    st_hp, // 3
    st_luck, // 4
    st_count,
};

enum item_grade {
    ig_none = 0,
    ig_normal, // 1
    ig_rare, // 2
    ig_unique, // 3
    ig_legendary, // 4
    ig_ancient, // 5
    ig_chaos, // 6
    ig_count
};

//@abi table ritem i64
struct ritem {
    uint64_t code;
    uint8_t type;
    uint8_t sub_type;
    uint8_t grade;
    uint8_t min_level;
    uint16_t powder;
    uint8_t stat1_type;
    uint8_t stat2_type;
    uint8_t stat3_type;
    uint16_t stat1;
    uint16_t stat2;
    uint16_t stat3;
    uint16_t stat1_rand_range;
    uint16_t stat2_rand_range;
    uint16_t stat3_rand_range;
    uint8_t stat2_reveal_rate;
    uint8_t stat3_reveal_rate;
    uint8_t mat1_code;
    uint8_t mat2_code;
    uint8_t mat3_code;
    uint8_t mat4_code;
    uint8_t mat1_count;
    uint8_t mat2_count;
    uint8_t mat3_count;
    uint8_t mat4_count;
    uint32_t rarity;
    uint8_t setid;
    uint8_t v1;
    uint16_t v2;
    uint32_t v3;

    ritem() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            ritem,
            (code)
            (type)
            (sub_type)
            (grade)
            (min_level)
            (powder)
            (stat1_type)
            (stat2_type)
            (stat3_type)
            (stat1)
            (stat2)
            (stat3)
            (stat1_rand_range)
            (stat2_rand_range)
            (stat3_rand_range)
            (stat2_reveal_rate)
            (stat3_reveal_rate)
            (mat1_code)
            (mat2_code)
            (mat3_code)
            (mat4_code)
            (mat1_count)
            (mat2_count)
            (mat3_count)
            (mat4_count)
            (rarity)
            (setid)
            (v1)
            (v2)
            (v3)
    )
};

typedef eosio::multi_index<N(ritem), ritem> ritem_table;
