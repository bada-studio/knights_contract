// 7 bytes
struct petrow {
    uint8_t code = 0;
    uint32_t count = 0;
    uint8_t level = 0;
    uint8_t knight = 0;
};

//@abi table pet i64
struct pet {
    name owner;
    std::vector<petrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            pet,
            (owner)
            (rows)
    )
};

typedef eosio::multi_index< N(pet), pet > pet_table;

//@abi table spet i64
struct spet {
    name owner;
    uint32_t season;
    std::vector<petrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            spet,
            (owner)
            (season)
            (rows)
    )
};

typedef eosio::multi_index< N(spet), spet > spet_table;
