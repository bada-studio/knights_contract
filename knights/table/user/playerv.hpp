// deprecated
//@abi table playerv i64
struct playerv {
    name owner;
    uint32_t from = 0;
    uint32_t to = 0;
    uint8_t referral = 0;
    uint8_t v4 = 0;
    uint16_t gift = 0;
    uint32_t asset = 0;
    uint32_t note = 0;
    uint32_t data = 0;
    uint32_t net = 0;
    uint32_t cpu = 0;

    playerv(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     playerv,
                     (owner)
                     (from)
                     (to)
                     (referral)
                     (v4)
                     (gift)
                     (asset)
                     (note)
                     (data)
                     (net)
                     (cpu)
                     )
};

typedef eosio::multi_index< N(playerv), playerv> playerv_table;

//@abi table playerv2 i64
struct playerv2 {
    name owner;
    uint32_t seed = 0;
    uint32_t next_deferred_time = 0;
    uint8_t referral = 0;
    uint8_t migrated = 2;
    uint16_t gift = 0;
    uint16_t ak1 = 0;
    uint16_t ak2 = 0;
    uint16_t ak3 = 0;
    uint16_t v1 = 0;
    uint16_t last_start_season = 0;
    uint16_t last_end_season = 0;
    uint32_t last_sell_time = 0;
    uint16_t sell_factor = 0;
    uint16_t rebrith_factor = 0;
    uint32_t block = 0;
    uint8_t floor_submit = 0;
    uint8_t itemevt = 0;
    uint16_t dquest_no = 0;
    uint16_t dq_p1 = 0;
    uint16_t dq_p2 = 0;
    uint16_t dq_p3 = 0;
    uint16_t dq_p4 = 0;
    uint16_t dq_p5 = 0;
    uint16_t v5 = 0;

    playerv2(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }

    void migrate0to2() {
        next_deferred_time = 0;
        v1 = 0;
        v5 = 0;
        last_sell_time = 0;
        sell_factor = 0;
        rebrith_factor = 0;
        floor_submit = 0;
        dquest_no = 0;
        itemevt = 0;
        dq_p1 = 0;
        dq_p2 = 0;
        dq_p3 = 0;
        dq_p4 = 0;
        dq_p5 = 0;
        migrated = 2;
    }

    void migrate1to2() {
        v1 = 0;
        v5 = 0;
        migrated = 2;
        itemevt = 0;
    }

    void clear_dungeon_quest_point() {
        dq_p1 = 0;
        dq_p2 = 0;
        dq_p3 = 0;
        dq_p4 = 0;
        dq_p5 = 0;
    }

    void clear_deferred_time() {
        next_deferred_time = 0;
    }

    void set_dungeon_quest_point(int mode, uint16_t point) {
        switch (mode) {
            case 1: dq_p1 = point; break;
            case 2: dq_p2 = point; break;
            case 3: dq_p3 = point; break;
            case 4: dq_p4 = point; break;
            case 5: dq_p5 = point; break;
            default: eosio_assert(0, "can not set to un-defined mode");
        }
    }

    uint16_t get_dungeon_quest_point(int mode) {
        switch (mode) {
            case 1: return dq_p1;
            case 2: return dq_p2;
            case 3: return dq_p3;
            case 4: return dq_p4;
            case 5: return dq_p5;
        }

        eosio_assert(0, "can not read from un-defined mode");
        return 0;
    }

    EOSLIB_SERIALIZE(
                     playerv2,
                     (owner)
                     (seed)
                     (next_deferred_time)
                     (referral)
                     (migrated)
                     (gift)
                     (ak1)
                     (ak2)
                     (ak3)
                     (v1)
                     (last_start_season)
                     (last_end_season)
                     (last_sell_time)
                     (sell_factor)
                     (rebrith_factor)
                     (block)
                     (floor_submit)
                     (itemevt)
                     (dquest_no)
                     (dq_p1)
                     (dq_p2)
                     (dq_p3)
                     (dq_p4)
                     (dq_p5)
                     (v5)
                     )
};

typedef eosio::multi_index< N(playerv2), playerv2> playerv2_table;
