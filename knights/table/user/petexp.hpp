// 7 bytes
struct petexprow {
    uint16_t code = 0;
    uint32_t start = 0;
    uint32_t end = 0;
    bool isback = false;
};

//@abi table petexp i64
struct petexp {
    name owner;
    uint32_t v1 = 0;
    uint32_t v2 = 0;
    std::vector<petexprow> rows;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            petexp,
            (owner)
            (v1)
            (v2)
            (rows)
    )
};

typedef eosio::multi_index< N(petexp), petexp > petexp_table;
