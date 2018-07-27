//@abi table airgrabstate i64
struct airgrabstate {
    uint64_t id = 0;
    asset grabbed;
    asset total;
    asset revenue;

    airgrabstate()
        : grabbed(0, S(4, BADA))
        , total(0, S(4, BADA))
        , revenue(0, S(4, BADA)) {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            airgrabstate,
            (id)
            (grabbed)
            (total)
            (revenue)
    )
};

typedef eosio::multi_index< N(airgrabstate), airgrabstate> airgrabstate_table;
