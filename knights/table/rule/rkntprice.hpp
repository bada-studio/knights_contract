//@abi table rkntprice i64
struct rkntprice {
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

typedef eosio::multi_index< N(rkntprice), rkntprice> rkntprice_table;
