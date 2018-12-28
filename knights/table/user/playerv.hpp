enum random_for {
    r4_rebirth,
    r4_craft,
    r4_petgacha,
    r4_petexp,
    r4_dungeon,
};

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
    uint32_t block = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;

    playerv2(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     playerv2,
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
                     (block)
                     (v1)
                     (v2)
                     )
};

typedef eosio::multi_index< N(playerv2), playerv2> playerv2_table;
