//@abi table rpetlv i64
struct rpetlv {
    uint64_t level = 0;
    uint16_t count = 0;
    uint16_t powder1 = 0;
    uint16_t powder2 = 0;
    uint16_t powder3 = 0;
    uint16_t powder4 = 0;
    uint16_t powder5 = 0;

    rpetlv() {
    }

    uint64_t primary_key() const {
        return level;
    }

    EOSLIB_SERIALIZE(
            rpetlv,
            (level)
            (count)
            (powder1)
            (powder2)
            (powder3)
            (powder4)
            (powder5)
    )
};

typedef eosio::multi_index< N(rpetlv), rpetlv> rpetlv_table;
