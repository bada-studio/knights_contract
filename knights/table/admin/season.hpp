struct seasoninfo {
    uint64_t start = 0; // start from
    uint32_t duration = 0; // event duration
    uint32_t petoffset = 0; // pet offset time
    uint32_t speed = 0; // game speed
    uint32_t init_powder = 0; // init dark magic water
    uint8_t stage = 0;
    uint8_t theme = 0;
    uint16_t max_record_count = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    uint64_t v3 = 0;
    asset spending_limit;
    std::vector<asset> rewards;
    std::vector<std::string> sponsors;

    uint64_t get_end() const {
        return start + duration;
    }

    bool is_in(uint64_t now) const {
        return (start <= now) && (now < get_end());
    }
};

struct seasonrecord {
    name owner; // account name
    uint32_t floor; 
    std::vector<knightrow> knights;
    std::vector<itemrow> equip;
    std::vector<petrow> adopted;
    bool paid = false; // for the dividened
};

struct seasonstate {
    uint32_t playercnt = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    std::vector<seasonrecord> records;
};

//@abi table season i64
struct season {
    uint64_t id = 0; // season id
    seasoninfo info;
    seasonstate state;
    uint64_t v1 = 0;
    uint64_t v2 = 0;

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(
            season,
            (id)
            (info)
            (state)
            (v1)
            (v2)
    )
};

typedef eosio::multi_index<N(season), season> season_table;
