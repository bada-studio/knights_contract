// 34 bytes
//@abi table skin i64
struct skin {
    uint64_t cid = 0;
    uint16_t code = 0;
    name player;

    skin() {
    }

    uint64_t primary_key() const {
        return cid;
    }

    EOSLIB_SERIALIZE(
            skin,
            (cid)
            (code)
            (player)
    )
};

typedef eosio::multi_index< N(skin), skin> skin_table;
