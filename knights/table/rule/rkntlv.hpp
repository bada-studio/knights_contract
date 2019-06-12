struct [[eosio::table]] rkntlv {
    uint64_t level = 0;
    uint32_t exp = 0;
    uint32_t powder = 0;

    rkntlv() {
    }

    uint64_t primary_key() const {
        return level;
    }

    EOSLIB_SERIALIZE(
            rkntlv,
            (level)
            (exp)
            (powder)
    )
};

typedef eosio::multi_index< "rkntlv"_n, rkntlv> rkntlv_table;
