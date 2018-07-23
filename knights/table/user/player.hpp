// 37 bytes
//@abi table player i64
struct player {
    name owner;  // 8
    uint8_t mat_ivn_up = 0;
    uint8_t item_ivn_up = 0;
    uint8_t current_stage = 0;
    uint32_t last_rebirth = 0;
    uint32_t powder = 0;
    uint16_t maxfloor = 0;

    player(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     player,
                     (owner)
                     (mat_ivn_up)
                     (item_ivn_up)
                     (current_stage)
                     (last_rebirth)
                     (powder)
                     (maxfloor)
                     )
};

typedef eosio::multi_index< N(player), player> player_table;
