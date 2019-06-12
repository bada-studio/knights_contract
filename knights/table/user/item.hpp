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

// todo is it working????????
//table item i64
//table sitem i64
struct [[eosio::table]] item {
    name owner;
    uint32_t last_id;
    std::vector<itemrow> rows;

    uint64_t primary_key() const {
        return owner.value;
    }

    EOSLIB_SERIALIZE(
            item,
            (owner)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< "item"_n, item > item_table;
typedef eosio::multi_index< "sitem"_n, item > sitem_table;