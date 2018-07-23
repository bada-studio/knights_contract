enum knight_type {
    kt_none = 0,
    kt_knight,
    kt_archer,
    kt_mage,
    kt_count
};

//@abi table rknt i64
struct rknt {
    uint64_t type = 0;
    uint16_t attack = 0;
    uint16_t hp = 0;
    uint16_t defense = 0;
    uint16_t luck = 0;
    uint16_t gattack = 0;
    uint16_t ghp = 0;
    uint16_t gdefense = 0;

    rknt() {
    }

    uint64_t primary_key() const {
        return type;
    }

    EOSLIB_SERIALIZE(
            rknt,
            (type)
            (attack)
            (hp)
            (defense)
            (luck)
            (gattack)
            (ghp)
            (gdefense)
    )
};

typedef eosio::multi_index< N(rknt), rknt> rknt_table;
