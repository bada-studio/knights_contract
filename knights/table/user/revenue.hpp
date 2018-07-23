enum contrat_type {
    ct_none = 0,
    ct_knight, // 1
    ct_material, // 2
    ct_item, // 3
    ct_mp, // 4
    ct_mat_iventory_up, // 5
    ct_item_iventory_up, // 6
    ct_mp_instant, // 7
};

// 42 bytes
struct selllog {
    uint32_t dt; 
    name buyer;
    uint8_t type; // contrat_type
    uint32_t pid;
    uint16_t code;
    uint32_t dna; // item only
    uint8_t level; // item only
    uint8_t exp; // item only
    asset price;
    uint8_t taxrate;
};

// 41 bytes
struct buylog {
    uint32_t dt = 0; 
    name seller;
    uint8_t type = 0; // contrat_type
    uint32_t pid = 0;
    uint16_t code = 0;
    uint32_t dna = 0; // item only
    uint8_t level = 0; // item only
    uint8_t exp = 0; // item only
    asset price;
};

// 58 + logs
//@abi table revenue i64
struct revenue {
    name owner;
    asset selling;
    asset spending;
    asset buying;
    uint32_t selling_count;
    uint32_t spending_count;
    uint16_t buying_count;
    std::vector<selllog> selllogs;
    std::vector<buylog> buylogs;

    revenue() 
        : selling(0, S(4, EOS))
        , spending(0, S(4, EOS))
        , buying(0, S(4, EOS)) {
    }

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            revenue,
            (owner)
            (selling)
            (spending)
            (buying)
            (selling_count)
            (spending_count)
            (buying_count)
            (selllogs)
            (buylogs)
    )
};

typedef eosio::multi_index< N(revenue), revenue> revenue_table;
