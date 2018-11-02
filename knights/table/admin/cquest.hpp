struct cquestdetail {
    uint16_t code = 0; // required item code
    uint16_t submit_limit = 0; // submit limit
    uint16_t submit_limit_pu = 0; // submit limit per user
    uint8_t score_from = 0; // required score 
    uint8_t score_to = 0; // required score 
    uint8_t level = 0; // required item level
    asset reward; // reward1
    asset reward2; // reward2
};

struct cquestrecord {
    name owner; // account name
    uint8_t submit_count = 0; // submit count
    bool paid = false; // for the dividened
};

struct csubquest {
    cquestdetail detail; // quest detail
    uint16_t total_submit_count = 0; // total submit count
    std::vector<cquestrecord> records; // each player's submit record
};

//@abi table cquest i64
struct cquest {
    uint64_t id = 0; // quest id
    uint16_t sponsor = 0; // sponsor code
    uint32_t start = 0; // start from
    uint32_t duration = 0; // event duration
    std::vector<csubquest> subquests; // sub quest

    uint64_t primary_key() const {
        return id;
    }

    uint32_t get_end() const {
        return start + duration;
    }

    bool is_cquest_period(uint32_t now) const {
        return (start <= now) && (now < get_end());
    }

    EOSLIB_SERIALIZE(
            cquest,
            (id)
            (sponsor)
            (start)
            (duration)
            (subquests)
    )
};


typedef eosio::multi_index<N(cquest), cquest> cquest_table;
