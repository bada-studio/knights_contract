enum rv_type {
    rv_knight = 0,
    rv_material_tax,
    rv_item_tax, 
    rv_mp, 
    rv_mat_iventory_up, 
    rv_item_iventory_up, 
    rv_coo_mat,
    rv_system,
};

//@abi table revenuedt i64
struct revenuedt {
    uint64_t id = 0;
    asset knight;
    asset material_tax;
    asset item_tax;
    asset mp;
    asset mat_iventory_up;
    asset item_iventory_up; 
    asset coo_mat;
    asset system;

    revenuedt()
        : knight(0, S(4, EOS))
        , material_tax(0, S(4, EOS))
        , item_tax(0, S(4, EOS))
        , mp(0, S(4, EOS))
        , mat_iventory_up(0, S(4, EOS))
        , item_iventory_up(0, S(4, EOS))
        , coo_mat(0, S(4, EOS))
        , system(0, S(4, EOS)) {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            revenuedt,
            (id)
            (knight)
            (material_tax)
            (item_tax)
            (mp)
            (mat_iventory_up)
            (item_iventory_up)
            (coo_mat)
            (system)
    )
};

typedef eosio::multi_index< N(revenuedt), revenuedt> revenuedt_table;
