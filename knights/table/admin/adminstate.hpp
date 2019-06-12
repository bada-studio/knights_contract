struct [[eosio::table]] adminstate {
    uint64_t id = 0;
    uint8_t pause = 0;
    asset revenue;
    asset dividend;
    asset expenses;
    asset investment;
    asset tradingvol;
    asset loss;
    asset va2; 
    name coo;
    uint32_t player_count = 0;
    uint64_t v1 = 0;
    uint32_t v2 = 0;
    uint64_t v3 = 0;
    uint64_t v4 = 0;
    uint64_t v5 = 0;

    adminstate()
        : revenue(0, eosio::symbol("EOS", 4))
        , expenses(0, eosio::symbol("EOS", 4))
        , dividend(0, eosio::symbol("EOS", 4))
        , investment(0, eosio::symbol("EOS", 4))
        , tradingvol(0, eosio::symbol("EOS", 4)) 
        , loss(0, eosio::symbol("EOS", 4)) 
        , va2(0, eosio::symbol("EOS", 4)) {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            adminstate,
            (id)
            (pause)
            (revenue)
            (dividend)
            (expenses)
            (investment)
            (tradingvol)
            (loss)
            (va2)
            (coo)
            (player_count)
            (v1)
            (v2)
            (v3)
            (v4)
            (v5)
    )
};

typedef eosio::multi_index< "adminstate"_n, adminstate> adminstate_table;


