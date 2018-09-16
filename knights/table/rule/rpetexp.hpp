//@abi table rpetexp i64
struct rpetexp {
    uint64_t level = 0;
    uint16_t mw1 = 0;
    uint16_t mw2 = 0;
    uint16_t mw3 = 0;
    uint16_t mw4 = 0;
    uint16_t mw5 = 0;
    uint16_t dr2 = 0;
    uint16_t dr3 = 0;
    uint16_t dr4 = 0;
    uint16_t dr5 = 0;

    rpetexp() {
    }

    uint64_t primary_key() const {
        return level;
    }

    uint16_t get_mw(int grade) const {
        switch (grade) {
            case pg_normal: return mw1;
            case pg_rare: return mw2;
            case pg_unique: return mw3;
            case pg_legendary: return mw4;
            case pg_ancient: return mw5;
        }
        return 0;
    }

    uint16_t get_drop_rate(int grade) const {
        switch (grade) {
            case pg_normal: return 100;
            case pg_rare: return dr2;
            case pg_unique: return dr3;
            case pg_legendary: return dr4;
            case pg_ancient: return dr5;
        }
        return 0;
    }

    EOSLIB_SERIALIZE(
            rpetexp,
            (level)
            (mw1)
            (mw2)
            (mw3)
            (mw4)
            (mw5)
            (dr2)
            (dr3)
            (dr4)
            (dr5)
    )
};

typedef eosio::multi_index< N(rpetexp), rpetexp> rpetexp_table;
