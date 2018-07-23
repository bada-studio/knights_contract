enum marketpid_type {
    mpidt_material = 1,
    mpidt_item = 2,
};

//@abi table marketpid i64
struct marketpid {
    uint64_t type = 0;
    uint64_t pid = 0;
    
    marketpid() {
    }

    uint64_t primary_key() const {
        return type;
    }

    EOSLIB_SERIALIZE(
            marketpid,
            (type)
            (pid)
    )
};

typedef eosio::multi_index< N(marketpid), marketpid> marketpid_table;
