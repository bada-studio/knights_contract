//@abi table rivnprice i64
struct rivnprice {
    uint64_t count;
    asset price;

    rivnprice() {
    }

    uint64_t primary_key() const {
        return count;
    }

    EOSLIB_SERIALIZE(
            rivnprice,
            (count)
            (price)
    )
};

typedef eosio::multi_index< N(rivnprice), rivnprice> rivnprice_table;
