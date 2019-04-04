struct seasonrecord {
    name owner; // account name
    uint32_t floor; 
    bool paid = false; // for the dividened
};

//@abi table season i64
struct season {
    uint64_t id = 0; // season id
    uint32_t start = 0; // start from
    uint32_t duration = 0; // event duration
    uint32_t speed = 0;
    uint32_t share = 0;
    uint32_t rankcnt = 0;
    uint32_t playercnt = 0;
    asset revenue;
    std::vector<seasonrecord> records;

    uint64_t primary_key() const {
        return id;
    }

    uint32_t get_end() const {
        return start + duration;
    }

    bool is_season_period(uint32_t now) const {
        return (start <= now) && (now < get_end());
    }

    EOSLIB_SERIALIZE(
            season,
            (id)
            (start)
            (duration)
            (speed)
            (share)
            (rankcnt)
            (playercnt)
            (revenue)
            (records)
    )
};

typedef eosio::multi_index<N(season), season> season_table;
