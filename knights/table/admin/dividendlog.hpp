struct dividendto {
    name to;
    asset amount;
};

struct [[eosio::table]] dividendlog {
    uint64_t no;
    uint32_t at;
    asset amount;
    std::vector<dividendto> to;

    dividendlog() 
        : amount(0, eosio::symbol("EOS", 4)) {
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

typedef eosio::multi_index< "dividendlog"_n, dividendlog> dividendlog_table;
