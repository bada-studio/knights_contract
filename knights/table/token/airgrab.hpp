// 37 bytes
//@abi table airgrab i64
struct airgrab {
    name owner;  // 8
    uint8_t grab = 0;
    uint32_t last_payment = 0;

    airgrab(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     airgrab,
                     (owner)
                     (grab)
                     (last_payment)
                     )
};

typedef eosio::multi_index< N(airgrab), airgrab> airgrab_table;
