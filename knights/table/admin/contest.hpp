struct conmission {
    uint16_t code = 0;
    uint8_t max_count = 0;
    uint8_t score_from = 0;
    uint8_t score_to = 0;
    uint8_t level = 0;
    uint32_t start = 0;
    uint32_t during = 0;
    asset reward;

    uint32_t get_end() const {
        return start + during;
    }

    bool is_contest_period(int32_t now) const {
        return (start <= now) && (now < get_end());
    }
};

// 10 bytes
struct contestrow {
    name owner;
    uint16_t count = 0;
    bool paid = false;
};

//@abi table contest i64
struct contest {
    uint64_t id = 0;
    uint8_t count = 0;
    conmission mission;
    std::vector<contestrow> rows;

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            contest,
            (id)
            (count)
            (mission)
            (rows)
    )
};


typedef eosio::multi_index<N(contest), contest> contest_table;
