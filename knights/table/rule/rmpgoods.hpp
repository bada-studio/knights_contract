//@abi table rmpgoods i64
struct rmpgoods {
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

typedef eosio::multi_index< N(rmpgoods), rmpgoods> rmpgoods_table;
