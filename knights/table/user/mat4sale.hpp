// 34 bytes
//@abi table mat4sale i64
struct mat4sale {
    uint64_t cid = 0;
    name player;
    asset price;
    uint16_t code = 0;

    mat4sale()
        : price(0, S(4, EOS)) {
    }

    uint64_t primary_key() const {
        return cid;
    }

    EOSLIB_SERIALIZE(
            mat4sale,
            (cid)
            (player)
            (price)
            (code)
    )
};

typedef eosio::multi_index< N(mat4sale), mat4sale> mat4sale_table;
