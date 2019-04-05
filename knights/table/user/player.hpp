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



// 37 bytes
//@abi table splayer i64
struct splayer {
    name owner;  // 8
    uint8_t mat_ivn_up = 0;
    uint8_t item_ivn_up = 0;
    uint32_t last_rebirth = 0;
    uint32_t dmw = 0; // dark magic water
    uint16_t maxfloor = 0;
    uint16_t rebrith_factor = 0;
    uint32_t season = 0;
    uint8_t received = 0;
    asset spending;
    uint64_t v1 = 0;

    splayer(name o = name())
    : owner(o)
    , spending(0, S(4, EOS)) {
    }
    
    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     splayer,
                     (owner)
                     (mat_ivn_up)
                     (item_ivn_up)
                     (last_rebirth)
                     (dmw)
                     (maxfloor)
                     (rebrith_factor)
                     (season)
                     (received)
                     (spending)
                     (v1)
                     )
};

typedef eosio::multi_index< N(splayer), splayer> splayer_table;
