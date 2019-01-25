struct dquestdetail {
    uint16_t mode = 0;
    uint16_t max_record_count = 0;
    asset reward; // reward1
    asset reward2; // reward2
};

struct dquestrecord {
    name owner; // account name
    uint32_t point; 
    bool paid = false; // for the dividened
};

struct dsubquest {
    dquestdetail detail; // quest detail
    std::vector<dquestrecord> records;
};

//@abi table dquest i64
struct dquest {
    uint64_t id = 0; // quest id
    uint16_t sponsor = 0; // sponsor code
    uint32_t start = 0; // start from
    uint32_t duration = 0; // event duration
    std::vector<dsubquest> subquests; // sub quest

    uint64_t primary_key() const {
        return id;
    }

    uint32_t get_end() const {
        return start + duration;
    }

    bool is_dquest_period(uint32_t now) const {
        return (start <= now) && (now < get_end());
    }

    EOSLIB_SERIALIZE(
            dquest,
            (id)
            (sponsor)
            (start)
            (duration)
            (subquests)
    )
};

typedef eosio::multi_index<N(dquest), dquest> dquest_table;
