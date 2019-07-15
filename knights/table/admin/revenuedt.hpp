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
        : knight(0, S(4, EOS))
        , material_tax(0, S(4, EOS))
        , item_tax(0, S(4, EOS))
        , mp(0, S(4, EOS))
        , mat_iventory_up(0, S(4, EOS))
        , item_iventory_up(0, S(4, EOS))
        , coo_mat(0, S(4, EOS))
        , system(0, S(4, EOS)) 
        , dmw(0, S(4, EOS)) 
        , v1(0, S(4, EOS)) 
        , v2(0, S(4, EOS)) 
        , v3(0, S(4, EOS)) 
        , v4(0, S(4, EOS)) {
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

typedef eosio::multi_index< N(revenuedt2), revenuedt2> revenuedt2_table;


//@abi table revenuewt i64
struct revenuewt {
    uint64_t id = 0;
    uint32_t count = 0;
    name wname;
    name waccount;
    asset revenue;
    asset shared;
    asset shared_for;

    revenuewt()
        : revenue(0, S(4, EOS))
        , shared(0, S(4, EOS))
        , shared_for(0, S(4, EOS)) {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            revenuewt,
            (id)
            (count)
            (wname)
            (waccount)
            (revenue)
            (shared)
            (shared_for)
    )
};

typedef eosio::multi_index< N(revenuewt), revenuewt> revenuewt_table;
