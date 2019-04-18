const uint8_t k_medal_death = 99;
const uint8_t k_medal_gold = 1;
const uint8_t k_medal_silver = 2;
const uint8_t k_medal_bronze = 3;
const uint8_t k_medal_master = 4;
const uint8_t k_medal_knight = 5;

struct medalrow {
    uint8_t id; 
    uint16_t count; 
};

//@abi table medal i64
struct medal {
    name owner;
    std::vector<medalrow> medals;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            medal,
            (owner)
            (medals)
    )
};

typedef eosio::multi_index<N(medal), medal> medal_table;
