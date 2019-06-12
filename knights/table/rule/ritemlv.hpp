struct [[eosio::table]] ritemlv {
    uint64_t level = 0;
    uint16_t count = 0;
    uint16_t bonus = 0;
    uint32_t powder1 = 0;
    uint32_t powder2 = 0;
    uint32_t powder3 = 0;
    uint32_t powder4 = 0;
    uint32_t powder5 = 0;
    uint32_t powder6 = 0;
    uint16_t rate = 0;
    uint32_t v1 = 0;

    ritemlv() {
    }

    uint64_t primary_key() const {
        return level;
    }

    EOSLIB_SERIALIZE(
            ritemlv,
            (level)
            (count)
            (bonus)
            (powder1)
            (powder2)
            (powder3)
            (powder4)
            (powder5)
            (powder6)
            (rate)
            (v1)
    )
};

typedef eosio::multi_index< "ritemlv"_n, ritemlv> ritemlv_table;
