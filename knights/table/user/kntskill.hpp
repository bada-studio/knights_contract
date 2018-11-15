struct kntskill {
    uint16_t code;
    uint8_t level;
    uint32_t v1;

    kntskill() {
    }    

    kntskill(uint16_t code, uint8_t level) {
        this->code = code;
        this->level = level;
        this->v1 = 0;
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

    kntskill& get_skill(int knt, int skillid) {
        if (knt == kt_knight) {
            for (int index = 0; index < knight_skill.size(); index++) {
                auto &item = knight_skill[index];
                if (item.code == skillid) {
                    return item;
                }
            }
        }

        if (knt == kt_archer) {
            for (int index = 0; index < archer_skill.size(); index++) {
                auto &item = archer_skill[index];
                if (item.code == skillid) {
                    return item;
                }
            }
        }

        if (knt == kt_mage) {
            for (int index = 0; index < mage_skill.size(); index++) {
                auto &item = mage_skill[index];
                if (item.code == skillid) {
                    return item;
                }
            }
        }

        return knight_skill[0];
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
