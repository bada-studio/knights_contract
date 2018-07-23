//@abi table rversion i64
struct rversion {
    name rule;
    uint16_t version = 0;

    rversion() {
    }

    uint64_t primary_key() const {
        return rule;
    }

    EOSLIB_SERIALIZE(
            rversion,
            (rule)
            (version)
    )
};

typedef eosio::multi_index< N(rversion), rversion> rversion_table;
