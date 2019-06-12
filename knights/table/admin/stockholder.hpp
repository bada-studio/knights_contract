struct [[eosio::table]] stockholder {
    name holder;
    uint16_t share = 0;

    uint64_t primary_key() const {
        return holder.value;
    }

    EOSLIB_SERIALIZE(
            stockholder,
            (holder)
            (share)
    )
};

typedef eosio::multi_index< "stockholder"_n, stockholder> stockholder_table;
