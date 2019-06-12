// 7 bytes
struct petrow {
    uint8_t code = 0;
    uint32_t count = 0;
    uint8_t level = 0;
    uint8_t knight = 0;
};

// todo check
// table pet i64
// table spet i64
struct [[eosio::table]] pet {
    name owner;
    std::vector<petrow> rows;

    uint64_t primary_key() const {
        return owner.value;
    }

    EOSLIB_SERIALIZE(
            pet,
            (owner)
            (rows)
    )
};

typedef eosio::multi_index< "pet"_n, pet > pet_table;
typedef eosio::multi_index< "spet"_n, pet > spet_table;
