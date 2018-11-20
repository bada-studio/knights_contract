// 7 bytes
struct kntskill {
    uint16_t code = 0;
    uint8_t level = 0;
    uint32_t v1 = 0;

    void set(uint16_t _code, uint8_t _level) {
        code = _code;
        level = _level;
        v1 = 0;
    }
};

//@abi table kntskills i64
struct kntskills {
    name owner;
    std::vector<kntskill> knight_skill;
    std::vector<kntskill> archer_skill;
    std::vector<kntskill> mage_skill;
    
    std::vector<kntskill>& get_skills(int knt) {
        if (knt == kt_knight) {
            return knight_skill;
        }
        
        if (knt == kt_archer) {
            return archer_skill;
        }        

        if (knt == kt_mage) {
            return mage_skill;
        }

        return knight_skill;
    }

    const std::vector<kntskill>& cget_skills(int knt) const {
        if (knt == kt_knight) {
            return knight_skill;
        }
        
        if (knt == kt_archer) {
            return archer_skill;
        }        

        if (knt == kt_mage) {
            return mage_skill;
        }

        return knight_skill;
    }

    kntskills() {
    }

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(
            kntskills,
            (owner)
            (knight_skill)
            (archer_skill)
            (mage_skill)
    )
};

typedef eosio::multi_index< N(kntskills), kntskills> kntskills_table;
