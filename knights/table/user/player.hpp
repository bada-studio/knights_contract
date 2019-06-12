struct [[eosio::table]] player {
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
        return owner.value;
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

typedef eosio::multi_index< "player"_n, player> player_table;

struct [[eosio::table]] splayer {
    name owner;  // 8
    uint8_t mat_ivn_up = 0;
    uint8_t item_ivn_up = 0;
    uint8_t current_stage = 0;
    uint32_t last_rebirth = 0;
    uint32_t powder = 0; 
    uint16_t maxfloor = 0;

    uint16_t rebrith_factor = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;

    splayer(name o = name())
    : owner(o) {
    }
    
    uint64_t primary_key() const {
        return owner.value;
    }
    
    EOSLIB_SERIALIZE(
                     splayer,
                     (owner)
                     (mat_ivn_up)
                     (item_ivn_up)
                     (current_stage)
                     (last_rebirth)
                     (powder)
                     (maxfloor)
                     (rebrith_factor)
                     (v1)
                     (v2)
                     )
};

typedef eosio::multi_index< "splayer"_n, splayer> splayer_table;
