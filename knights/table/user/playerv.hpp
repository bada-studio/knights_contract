enum random_for {
    r4_rebirth,
    r4_craft,
    r4_petgacha,
    r4_petexp,
    r4_dungeon,
};

// 37 bytes
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
