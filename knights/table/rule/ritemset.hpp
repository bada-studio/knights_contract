// 7 bytes
struct ritemsetrow {
    uint16_t setid = 0;
    uint8_t count = 0;
    std::vector<uint16_t> codes;
    uint8_t stat1_type;
    uint8_t stat2_type;
    uint8_t stat3_type;
    uint16_t stat1;
    uint16_t stat2;
    uint16_t stat3;
};

//@abi table ritemset i64
struct ritemset {
    name owner;
    std::vector<ritemsetrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            ritemset,
            (owner)
            (rows)
    )
};

typedef eosio::multi_index< N(ritemset), ritemset > ritemset_table;
