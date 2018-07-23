struct dividendto {
    name to;
    asset amount;
};

//@abi table dividendlog i64
struct dividendlog {
    uint64_t no;
    uint32_t at;
    asset amount;
    std::vector<dividendto> to;

    dividendlog() 
        : amount(0, S(4, EOS)) {
    }

    uint64_t primary_key() const {
        return no;
    }

    EOSLIB_SERIALIZE(
            dividendlog,
            (no)
            (at)
            (amount)
            (to)
    )
};

typedef eosio::multi_index< N(dividendlog), dividendlog> dividendlog_table;
