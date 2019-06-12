struct [[eosio::table]] expenseslog {
    uint64_t no = 0;
    uint32_t at = 0;
    asset amount;
    name to;
    std::string memo;

    expenseslog() 
        : amount(0, eosio::symbol("EOS", 4)) {
    }

    uint64_t primary_key() const {
        return no;
    }

    EOSLIB_SERIALIZE(
            expenseslog,
            (no)
            (at)
            (amount)
            (to)
            (memo)
    )
};

typedef eosio::multi_index< "expenseslog"_n, expenseslog> expenseslog_table;
