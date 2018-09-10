//@abi table rpetexp i64
struct rpetexp {
    uint64_t level = 0;
    uint16_t mw1 = 0;
    uint16_t mw2 = 0;
    uint16_t mw3 = 0;
    uint16_t mw4 = 0;
    uint16_t mw5 = 0;

    rpetexp() {
    }

    uint64_t primary_key() const {
        return level;
    }

    EOSLIB_SERIALIZE(
            rpetexp,
            (level)
            (mw1)
            (mw2)
            (mw3)
            (mw4)
            (mw5)
    )
};

typedef eosio::multi_index< N(rpetexp), rpetexp> rpetexp_table;
