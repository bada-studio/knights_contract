//@abi table rmpgoods i64
struct rmpgoods {
    uint32_t pid;
    uint8_t type;
    uint8_t v1;
    uint16_t v2;
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
            (type)
            (v1)
            (v2)
            (powder)
            (price)
    )
};

typedef eosio::multi_index< N(rmpgoods), rmpgoods> rmpgoods_table;
