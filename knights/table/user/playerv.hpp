enum random_for {
    r4_rebirth,
    r4_craft,
    r4_petgacha
};

// 37 bytes
//@abi table playerv i64
struct playerv {
    name owner;
    uint32_t v1 = 0;
    uint32_t v2 = 0;
    uint8_t v3 = 0;
    uint8_t v4 = 0;
    uint16_t v5 = 0;
    uint32_t v6 = 0;
    uint32_t v7 = 0;
    uint32_t v8 = 0;
    uint32_t v9 = 0;
    uint32_t v10 = 0;

    playerv(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     playerv,
                     (owner)
                     (v1)
                     (v2)
                     (v3)
                     (v4)
                     (v5)
                     (v6)
                     (v7)
                     (v8)
                     (v9)
                     (v10)
                     )
};

typedef eosio::multi_index< N(playerv), playerv> playerv_table;
