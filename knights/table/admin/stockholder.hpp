//@abi table stockholder i64
struct stockholder {
    name holder;
    uint16_t share = 0;

    uint64_t primary_key() const {
        return holder;
    }

    EOSLIB_SERIALIZE(
            stockholder,
            (holder)
            (share)
    )
};

typedef eosio::multi_index< N(stockholder), stockholder> stockholder_table;
