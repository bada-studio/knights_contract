enum gift_type {
    gt_magic_water = 0,
};

enum gift_icon_type {
    git_gift = 0,
    git_sorry = 0,
};

//@abi table gift i64
struct gift {
    uint64_t no = 0;
    uint8_t type = 0;
    uint8_t icontype = 0;
    uint16_t amount = 0;
    uint32_t to = 0;

    gift() {
    }

    uint64_t primary_key() const {
        return no;
    }

    EOSLIB_SERIALIZE(
            gift,
            (no)
            (type)
            (icontype)
            (amount)
            (to)
    )
};

typedef eosio::multi_index< N(gift), gift> gift_table;
