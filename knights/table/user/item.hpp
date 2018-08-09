// 16 bytes
struct itemrow {
    uint32_t id = 0;
    uint16_t code = 0;
    uint8_t knight = 0;
    uint8_t level = 0;
    uint8_t exp = 0;
    uint32_t dna = 0;
    uint32_t saleid = 0;
};

//@abi table item i64
struct item {
    name owner;
    uint32_t last_id;
    std::vector<itemrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            item,
            (owner)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< N(item), item > item_table;
