//@abi table rstage i64
struct rstage {
    uint64_t id = 0;
    uint8_t lvfrom = 0;
    uint8_t drop_rate = 0;
    uint8_t nature_drop_rate = 0;
    uint8_t steel_drop_rate = 0;
    uint8_t bone_drop_rate = 0;
    uint8_t skin_drop_rate = 0;
    uint8_t mineral_drop_rate = 0;

    rstage() {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            rstage,
            (id)
            (lvfrom)
            (drop_rate)
            (nature_drop_rate)
            (steel_drop_rate)
            (bone_drop_rate)
            (skin_drop_rate)
            (mineral_drop_rate)
    )
};

typedef eosio::multi_index< N(rstage), rstage> rstage_table;
