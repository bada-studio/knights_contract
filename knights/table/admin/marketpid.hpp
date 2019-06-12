enum marketpid_type {
    mpidt_material = 1,
    mpidt_item = 2,
};

struct [[eosio::table]] marketpid {
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

typedef eosio::multi_index< "marketpid"_n, marketpid> marketpid_table;
