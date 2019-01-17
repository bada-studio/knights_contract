//@abi table adminstate i64
struct adminstate {
    uint64_t id = 0;
    uint8_t pause = 0;
    asset revenue;
    asset dividend;
    asset expenses;
    asset investment;
    asset tradingvol;
    asset va1;
    asset va2;
    name coo;
    uint32_t player_count = 0;
    uint64_t floor_sum = 0;
    uint32_t floor_sumbit_count = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    uint64_t v3 = 0;

    adminstate()
        : revenue(0, S(4, EOS))
        , expenses(0, S(4, EOS))
        , dividend(0, S(4, EOS))
        , investment(0, S(4, EOS))
        , tradingvol(0, S(4, EOS)) 
        , va1(0, S(4, EOS)) 
        , va2(0, S(4, EOS)) {
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
            (va1)
            (va2)
            (coo)
            (player_count)
            (floor_sum)
            (floor_sumbit_count)
            (v1)
            (v2)
            (v3)
    )
};

typedef eosio::multi_index< N(adminstate), adminstate> adminstate_table;


