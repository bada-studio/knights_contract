struct [[eosio::table]] rkntprice {
    uint64_t count = 0;
    asset price;

    rkntprice() {
    }

    uint64_t primary_key() const {
        return count;
    }

    EOSLIB_SERIALIZE(
            rkntprice,
            (count)
            (price)
    )
};

typedef eosio::multi_index< "rkntprice"_n, rkntprice> rkntprice_table;
