//@abi table novaevt i64
struct novaevt {
    uint64_t id = 0;
    uint32_t total = 0;
    uint32_t remain = 0;
    uint32_t amount = 0;

    novaevt() {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            novaevt,
            (id)
            (total)
            (remain)
            (amount)
    )
};

typedef eosio::multi_index< N(novaevt), novaevt> novaevt_table;
