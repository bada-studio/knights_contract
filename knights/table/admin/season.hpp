struct seasoncq {
    uint16_t code = 0;
    uint8_t level = 1;
    asset reward;
};

struct seasonsp {
    std::string sponsor;
    std::string link;
};

struct seasoninfo {
    uint64_t start = 0; // start from
    uint32_t duration = 0; // event duration
    uint32_t speed = 0; // game speed
    uint32_t init_powder = 0; // init dark magic water
    uint8_t stage = 0;
    uint8_t theme = 0;
    seasoncq quest;
    asset reward;
    uint8_t rewardcnt = 0;
    uint8_t rankcnt = 0;
    uint8_t min_reward_powder = 0;
    uint16_t max_reward_powder = 0;
    uint8_t force_skin = 0;
    bool opt_no_dmw = false;
    bool opt_no_pet = false;
    uint8_t opt_mat_shop = 0;
    uint8_t opt_item_shop = 0;
    uint8_t v1 = 0;
    uint16_t v2 = 0;
    uint32_t v3 = 0;
    uint64_t v4 = 0;
    std::vector<seasonsp> sponsors;

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
    uint64_t v1; 
    bool paid = false; // for the dividened
};

struct seasonstate {
    uint32_t playercnt = 0;
    name cqwinner;
    asset dmw;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    std::vector<seasonrecord> records;

    int get_rank(name from) const {
        for (int index = 0; index < records.size(); index++) {
            if (records[index].owner == from) {
                return index + 1;
            }
        }

        return 0;
    }
};

struct [[eosio::table]] season {
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

typedef eosio::multi_index<"season"_n, season> season_table;
