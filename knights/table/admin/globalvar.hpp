struct [[eosio::table]] globalvar {
    uint64_t id = 0;
    uint64_t floor_sum = 0;
    uint32_t floor_submit_count = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    uint64_t v3 = 0;
    uint64_t v4 = 0;
    uint64_t v5 = 0;

    globalvar() {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            globalvar,
            (id)
            (floor_sum)
            (floor_submit_count)
            (v1)
            (v2)
            (v3)
            (v4)
            (v5)
    )
};

typedef eosio::multi_index< "globalvar"_n, globalvar> globalvar_table;


