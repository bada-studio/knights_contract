struct seasonrecord {
    name owner; // account name
    uint32_t floor; 
    std::vector<knightrow> knights;
    std::vector<itemrow> equip;
    std::vector<petrow> adopted;
    bool paid = false; // for the dividened
};

//@abi table season i64
struct season {
    uint64_t id = 0; // season id
    uint64_t start = 0; // start from
    uint32_t duration = 0; // event duration
    uint32_t speed = 0; // game speed
    uint32_t init_dmw = 0; // init dark magic water
    asset spending_limit;
    std::vector<asset> rewards;
    std::vector<std::string> sponsors;
    uint32_t playercnt = 0;
    std::vector<seasonrecord> records;

    season() 
        : spending_limit(0, S(4, EOS)) {

    }

    uint64_t primary_key() const {
        return id;
    }

    uint64_t get_end() const {
        return start + duration;
    }

    bool is_in(uint64_t now) const {
        return (start <= now) && (now < get_end());
    }

    EOSLIB_SERIALIZE(
            season,
            (id)
            (start)
            (duration)
            (speed)
            (init_dmw)
            (spending_limit)
            (rewards)
            (sponsors)
            (playercnt)
            (records)
    )
};

typedef eosio::multi_index<N(season), season> season_table;
