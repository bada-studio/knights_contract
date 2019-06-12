struct [[eosio::table]] mat4sale {
    uint64_t cid = 0;
    name player;
    asset price;
    uint16_t code = 0;

    mat4sale()
        : price(0, eosio::symbol("EOS", 4)) {
    }

    uint64_t primary_key() const {
        return cid;
    }

    EOSLIB_SERIALIZE(
            mat4sale,
            (cid)
            (player)
            (price)
            (code)
    )
};

typedef eosio::multi_index< "mat4sale"_n, mat4sale> mat4sale_table;
