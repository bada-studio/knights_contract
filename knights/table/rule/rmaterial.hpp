enum material_type {
    mt_none = 0,
    mt_nature, // 1
    mt_iron, // 2
    mt_bone, // 3
    mt_skin, // 4
    mt_mineral, // 5
};

//@abi table rmaterial i64
struct rmaterial {
    uint64_t code = 0;
    uint8_t type = 0;
    uint8_t grade = 0;
    uint32_t relative_drop_rate = 0;
    uint16_t powder = 0;

    rmaterial() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rmaterial,
            (code)
            (type)
            (grade)
            (relative_drop_rate)
            (powder)
    )
};

typedef eosio::multi_index<N(rmaterial), rmaterial> rmaterial_table;
