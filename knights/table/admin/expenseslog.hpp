//@abi table expenseslog i64
struct expenseslog {
    uint64_t no = 0;
    uint32_t at = 0;
    asset amount;
    name to;
    std::string memo;

    expenseslog() 
        : amount(0, S(4, EOS)) {
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

typedef eosio::multi_index< N(expenseslog), expenseslog> expenseslog_table;
