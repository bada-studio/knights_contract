// 10 bytes
struct matrow {
    uint32_t id = 0;
    uint16_t code = 0;
    uint32_t saleid = 0;
};

//@abi table material i64
struct material {
    name owner;
    uint32_t last_id;
    std::vector<matrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            material,
            (owner)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< N(material), material> material_table;


//@abi table smaterial i64
struct smaterial {
    name owner;
    uint32_t season;
    uint32_t last_id;
    std::vector<matrow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            smaterial,
            (owner)
            (season)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< N(smaterial), smaterial> smaterial_table;
