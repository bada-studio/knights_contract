struct building {
    uint32_t id = 0;
    uint16_t code = 0;
    uint8_t level = 0;
    uint16_t hp = 0;
    uint8_t x = 0;
    uint8_t y = 0;
};

//@abi table village i64
struct village {
    name owner;
    uint32_t last_id;

    // ordered by (id or position)?
    // it need fast search O(NlogN)
    std::vector<building> buildings;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            village,
            (owner)
            (last_id)
            (buildings)
    )
};

typedef eosio::multi_index< N(village), village > village_table;
