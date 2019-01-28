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

enum deferred_trx_type {
    dtt_rebirth = 0,
    dtt_craft,
    dtt_pexpreturn,
    dtt_petgacha,
    dtt_dgclear,
    dtt_itemlvup,
};

//@abi table playerv2 i64
struct playerv2 {
    name owner;
    uint32_t seed = 0;
    uint32_t deferred_rebirth = 0;
    uint8_t referral = 0;
    uint8_t migrated = 0;
    uint16_t gift = 0;
    uint32_t deferred_craft = 0;
    uint32_t deferred_pexpreturn = 0;
    uint32_t deferred_petgacha = 0;
    uint32_t deferred_dgclear = 0;
    uint32_t deferred_itemlvup = 0;
    uint32_t block = 0;
    uint8_t floor_submit = 0;
    uint8_t v1 = 0;
    uint16_t dquest_no = 0;
    uint16_t rebrith_factor = 0;
    uint16_t dq_p1 = 0;
    uint16_t dq_p2 = 0;
    uint16_t dq_p3 = 0;
    uint16_t dq_p4 = 0;
    uint16_t dq_p5 = 0;

    playerv2(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }

    void migrate() {
        deferred_rebirth = 0;
        deferred_craft = 0;
        deferred_pexpreturn = 0;
        deferred_petgacha = 0;
        deferred_dgclear = 0;
        deferred_itemlvup = 0;
        floor_submit = 0;
        rebrith_factor = 0;
        dquest_no = 0;
        migrated = 1;
        dq_p1 = 0;
        dq_p2 = 0;
        dq_p3 = 0;
        dq_p4 = 0;
        dq_p5 = 0;
    }

    void clear_dungeon_quest_point() {
        dq_p1 = 0;
        dq_p2 = 0;
        dq_p3 = 0;
        dq_p4 = 0;
        dq_p5 = 0;
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

    uint32_t get_deferred_time(deferred_trx_type type) const {
        switch (type) {
            case dtt_rebirth: return deferred_rebirth;
            case dtt_craft: return deferred_craft;
            case dtt_pexpreturn: return deferred_pexpreturn;
            case dtt_petgacha: return deferred_petgacha;
            case dtt_dgclear: return deferred_dgclear;
            case dtt_itemlvup: return deferred_itemlvup;
        }

        return 0;
    }

    void set_deferred_time(deferred_trx_type type, uint32_t value) {
        switch (type) {
            case dtt_rebirth: deferred_rebirth = value; break;
            case dtt_craft: deferred_craft = value; break;
            case dtt_pexpreturn: deferred_pexpreturn = value; break;
            case dtt_petgacha: deferred_petgacha = value; break;
            case dtt_dgclear: deferred_dgclear = value; break;
            case dtt_itemlvup: deferred_itemlvup = value; break;
        }
    }

    EOSLIB_SERIALIZE(
                     playerv2,
                     (owner)
                     (seed)
                     (deferred_rebirth)
                     (referral)
                     (migrated)
                     (gift)
                     (deferred_craft)
                     (deferred_pexpreturn)
                     (deferred_petgacha)
                     (deferred_dgclear)
                     (deferred_itemlvup)
                     (block)
                     (floor_submit)
                     (v1)
                     (dquest_no)
                     (rebrith_factor)
                     (dq_p1)
                     (dq_p2)
                     (dq_p3)
                     (dq_p4)
                     (dq_p5)
                     )
};

typedef eosio::multi_index< N(playerv2), playerv2> playerv2_table;
