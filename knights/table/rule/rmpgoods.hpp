struct [[eosio::table]] rmpgoods {
    uint64_t pid;
    uint32_t powder;
    asset price;

    rmpgoods() {
    }

    uint64_t primary_key() const {
        return pid;
    }

    EOSLIB_SERIALIZE(
            rmpgoods,
            (pid)
            (powder)
            (price)
    )
};

typedef eosio::multi_index< "rmpgoods"_n, rmpgoods> rmpgoods_table;
