//@abi table item4sale i64
// 40 bytes
struct item4sale {
    uint64_t cid = 0;
    name player;
    asset price;
    uint16_t code = 0;
    uint32_t dna = 0;
    uint8_t level = 0;
    uint8_t exp = 0;

    uint64_t primary_key() const {
        return cid;
    }

    item4sale()
        : price(0, S(4, EOS)) {
    }

    EOSLIB_SERIALIZE(
            item4sale,
            (cid)
            (player)
            (price)
            (code)
            (dna)
            (level)
            (exp)
    )
};

typedef eosio::multi_index< N(item4sale), item4sale> item4sale_table;
