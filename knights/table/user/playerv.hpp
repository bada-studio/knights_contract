// 37 bytes
//@abi table playerv i64
struct playerv {
    name owner;  // 8
    uint64_t seed = 0;
    uint32_t v1 = 0;
    uint32_t v2 = 0;
    uint32_t v3 = 0;
    uint32_t v4 = 0;
    uint32_t v5 = 0;
    uint32_t v6 = 0;

    playerv(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     playerv,
                     (owner)
                     (seed)
                     (v1)
                     (v2)
                     (v3)
                     (v4)
                     (v5)
                     (v6)
                     )
};

typedef eosio::multi_index< N(playerv), playerv> playerv_table;
