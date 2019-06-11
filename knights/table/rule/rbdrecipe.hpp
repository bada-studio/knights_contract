enum building_type {
    bt_none = 0,
    bt_obstacle,
    bt_magic_water,
    bt_knight_house,
    bt_archer_house,
    bt_mage_house,
    bt_deco,
};

enum ingredient_type {
    igt_none,
    igt_material,
    igt_item,
};


//@abi table rbdrecipe i64
struct rbdrecipe {
    uint64_t code = 0;
    uint8_t level = 0;
    uint16_t hp = 0;
    uint16_t mw = 0;
    bool buildable = 0;
    uint8_t type1 = 0;
    uint16_t ig_code1 = 0;
    uint8_t ig_count1 = 0;
    uint8_t type2 = 0;
    uint16_t ig_code2 = 0;
    uint8_t ig_count2 = 0;
    uint8_t type3 = 0;
    uint16_t ig_code3 = 0;
    uint8_t ig_count3 = 0;

    rbdrecipe() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rbdrecipe,
            (code)
            (level)
            (hp)
            (mw)
            (buildable)
            (type1)
            (ig_code1)
            (ig_count1)
            (type2)
            (ig_code2)
            (ig_count2)
            (type3)
            (ig_code3)
            (ig_count3)
    )
};

typedef eosio::multi_index< N(rbdrecipe), rbdrecipe> rbdrecipe_table;
