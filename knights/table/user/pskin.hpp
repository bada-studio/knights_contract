// 16 bytes
struct pskinrow {
    uint32_t id = 0;
    uint16_t code = 0;
};

//@abi table pskin i64
struct pskin {
    name owner;
    std::vector<pskinrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            pskin,
            (owner)
            (rows)
    )
};

typedef eosio::multi_index< N(pskin), pskin > pskin_table;
