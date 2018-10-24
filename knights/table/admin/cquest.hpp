struct cquestinfo {
    uint16_t code = 0;
    uint8_t max_count = 0;
    uint8_t score_from = 0;
    uint8_t score_to = 0;
    uint8_t level = 0;
    uint16_t cooltime_min = 0;
    uint32_t start = 0;
    uint32_t during = 0;
    asset reward;

    uint32_t get_end() const {
        return start + during;
    }

    bool is_cquest_period(uint32_t now) const {
        return (start <= now) && (now < get_end());
    }
};

// 10 bytes
struct cquestrow {
    name owner;
    uint32_t at = 0;
    uint16_t count = 0;
    bool paid = false;
};

//@abi table cquest i64
struct cquest {
    uint64_t id = 0;
    uint8_t count = 0;
    cquestinfo info;
    std::vector<cquestrow> rows;

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            cquest,
            (id)
            (count)
            (info)
            (rows)
    )
};


typedef eosio::multi_index<N(cquest), cquest> cquest_table;
