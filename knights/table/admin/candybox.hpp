//@abi table candybox i64
struct candybox {
    uint64_t id = 0;
    uint32_t total = 0;
    uint32_t remain = 0;
    uint32_t amount = 0;

    candybox() {
    }

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            candybox,
            (id)
            (total)
            (remain)
            (amount)
    )
};

typedef eosio::multi_index< N(candybox), candybox> candybox_table;
