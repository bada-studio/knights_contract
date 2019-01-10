
//@abi table tklog i64
struct tklog {
    name owner;
    uint16_t count = 0;
    
    tklog() {
    }

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            tklog,
            (owner)
            (count)
    )
};

typedef eosio::multi_index< N(tklog), tklog> tklog_table;
