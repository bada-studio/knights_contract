//@abi table itemevt i64
struct itemevt {
    uint64_t id = 0;
    uint32_t code = 0;
    uint64_t from = 0;
    uint32_t duration = 0;

    itemevt() {
    }

    uint64_t primary_key() const {
        return id;
    }

    uint64_t end() const {
        return from + duration;
    }

    bool is_in(uint64_t at) const {
        return (from <= at && at < from + duration);
    }

    EOSLIB_SERIALIZE(
            itemevt,
            (id)
            (code)
            (from)
            (duration)
    )
};

typedef eosio::multi_index< N(itemevt), itemevt> itemevt_table;
