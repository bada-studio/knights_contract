//@abi table adminstate i64
struct adminstate {
    uint64_t id = 0;
    uint8_t pause = 0;
    asset revenue;
    asset dividend;
    asset expenses;
    asset investment;
    asset tradingvol;
    uint32_t player_count = 0;
    name coo;

    adminstate()
        : revenue(0, S(4, EOS))
        , expenses(0, S(4, EOS))
        , dividend(0, S(4, EOS))
        , investment(0, S(4, EOS))
        , tradingvol(0, S(4, EOS)) {
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
            (player_count)
            (coo)
    )
};

typedef eosio::multi_index< N(adminstate), adminstate> adminstate_table;
