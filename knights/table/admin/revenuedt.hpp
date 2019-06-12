enum rv_type {
    rv_knight = 0,
    rv_material_tax,
    rv_item_tax, 
    rv_mp, 
    rv_mat_iventory_up, 
    rv_item_iventory_up, 
    rv_skin,
    rv_system,
    rv_dmw, 
};

struct [[eosio::table]] revenuedt {
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
        : knight(0, eosio::symbol("EOS", 4))
        , material_tax(0, eosio::symbol("EOS", 4))
        , item_tax(0, eosio::symbol("EOS", 4))
        , mp(0, eosio::symbol("EOS", 4))
        , mat_iventory_up(0, eosio::symbol("EOS", 4))
        , item_iventory_up(0, eosio::symbol("EOS", 4))
        , coo_mat(0, eosio::symbol("EOS", 4))
        , system(0, eosio::symbol("EOS", 4)) {
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

typedef eosio::multi_index<"tablename"_n, revenuedt> revenuedt_table;

//@abi table revenuedt2 i64
struct revenuedt2 {
    uint64_t id = 0;
    asset knight;
    asset material_tax;
    asset item_tax;
    asset mp;
    asset mat_iventory_up;
    asset item_iventory_up; 
    asset coo_mat;
    asset system;
    asset dmw;
    asset v1;
    asset v2;
    asset v3;
    asset v4;

    revenuedt2()
        : knight(0, eosio::symbol("EOS", 4))
        , material_tax(0, eosio::symbol("EOS", 4))
        , item_tax(0, eosio::symbol("EOS", 4))
        , mp(0, eosio::symbol("EOS", 4))
        , mat_iventory_up(0, eosio::symbol("EOS", 4))
        , item_iventory_up(0, eosio::symbol("EOS", 4))
        , coo_mat(0, eosio::symbol("EOS", 4))
        , system(0, eosio::symbol("EOS", 4)) 
        , dmw(0, eosio::symbol("EOS", 4)) 
        , v1(0, eosio::symbol("EOS", 4)) 
        , v2(0, eosio::symbol("EOS", 4)) 
        , v3(0, eosio::symbol("EOS", 4)) 
        , v4(0, eosio::symbol("EOS", 4)) {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            revenuedt2,
            (id)
            (knight)
            (material_tax)
            (item_tax)
            (mp)
            (mat_iventory_up)
            (item_iventory_up)
            (coo_mat)
            (system)
            (dmw)
            (v1)
            (v2)
            (v3)
            (v4)
    )
};

typedef eosio::multi_index< "revenuedt2"_n, revenuedt2> revenuedt2_table;
