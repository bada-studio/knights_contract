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
    uint32_t rebrith_factor = 0;
    uint32_t dquest_point = 0;
    uint32_t v2 = 0;

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
        dquest_point = 0;
        migrated = 1;
        v1 = 0;
        v2 = 0;
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
                     (dquest_point)
                     (v2)
                     )
};

typedef eosio::multi_index< N(playerv2), playerv2> playerv2_table;
