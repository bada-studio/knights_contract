#pragma once

class pet_control : public control_base {
private:
    account_name self;

    pet_table pets;
    rule_controller<rpet, rpet_table> rpet_controller;
    rule_controller<rpetlv, rpetlv_table> rpetlv_controller;
    player_control &player_controller;
    saleslog_control &saleslog_controller;

    std::vector<petrow> empty_petrows;

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    pet_control(account_name _self,
                player_control &_player_controller,
                saleslog_control &_saleslog_controller)
            : self(_self)
            , pets(_self, _self)
            , rpet_controller(_self, N(pet))
            , rpetlv_controller(_self, N(petlv))
            , player_controller(_player_controller)
            , saleslog_controller(_saleslog_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    pet_table::const_iterator find(name from) {
        return pets.find(from);
    }

    const std::vector<petrow>& get_pets(name from) {
        auto iter = pets.find(from);
        return get_pets(iter);
    }

    const std::vector<petrow>& get_pets(pet_table::const_iterator iter) {
        if (iter != pets.cend()) {
            return iter->rows;
        }

        return empty_petrows;
    }

    void add_pet(name from, int code) {
        petrow row;
        row.code = code;
        row.level = 1;
        row.count = 1;

        auto iter = pets.find(from);
        if (iter == pets.cend()) {
            pets.emplace(self, [&](auto& pet){
                pet.owner = from;
                pet.rows.push_back(row);
            });
        } else {
            pets.modify(iter, self, [&](auto& pet){
                bool exist = false;
                for (int index = 0; index < pet.rows.size(); index++) {
                    if (pet.rows[index].code == code) {
                        pet.rows[index].count++;
                        exist = true;
                        break;
                    }
                }

                if (exist == false) {
                    pet.rows.push_back(row);
                }
            });
        }
    }


    /// @brief
    /// Pet stat calculation logic
    /// Add the stats of the pet attached at the knight stats calculation.
    /// @param stat
    /// Knight stat
    /// @param name
    /// Account name for the knight owner
    /// @param knight
    /// Target to be calculated
    void apply_pet_stats(knight_stats &stat, name from, uint64_t knight) {
        auto &pet_rule = rpet_controller.get_table();
        auto &pets = get_pets(from);

        for (int index = 0; index < pets.size(); index++) {
            auto &pet = pets[index];
            if (pet.knight != knight) {
                continue;
            }

            auto rule = pet_rule.find(pet.code);
            assert_true(rule != pet_rule.cend(), "could not find pet rule");

            int level = pet.level;
            int stat1 = rule->stat1 + (rule->stat1_up_per_level * (level - 1));
            add_stat(stat, (stat_type)rule->stat1_type, stat1);

            if (rule->stat2_type > 0) {
                int stat2 = rule->stat2 + (rule->stat2_up_per_level * (level - 1));
                add_stat(stat, (stat_type)rule->stat2_type, stat2);
            }

            if (rule->stat3_type > 0) {
                int stat3 = rule->stat3 + (rule->stat3_up_per_level * (level - 1));
                add_stat(stat, (stat_type)rule->stat3_type, stat3);
            }
        }
    }

    // actions
    //-------------------------------------------------------------------------
    /// @brief
    /// Pet gacha. Run count times.
    /// Magic powder is consumed. If the powder is insufficient, it fails.
    /// @param from
    /// Player who requested gocha
    /// @param type
    /// low_class or high_class, @see pet_gacha_type
    /// @param count
    /// Gocha request count
    void petgacha(name from, uint16_t type, uint8_t count) {
        require_auth(from);
        assert_true(type > 0 && type < pgt_count, "invalid gacha type");
        assert_true(count > 0 && count < 10, "invalid count");

        int powder;
        if (type == pgt_low_class) {
            powder = kv_pet_gacha_low_price;
        } else {
            powder = kv_pet_gacha_high_price;
        }

        powder *= count;
        auto player = player_controller.get_player(from);
        assert_true(!player_controller.is_empty_player(player), "could not find player");
        assert_true(powder <= player->powder, "not enough powder");

        if (powder > 0) {
            player_controller.decrease_powder(player, powder);
        }

        //@ warning for the performance issue, drop rates are hard coded here,
        // be careful for the data sync issue with pet rule.
        const uint32_t sum_low = 233600;
        const uint32_t sum_high = 28960;
        const uint32_t rare_start = 8;
        const uint32_t legend_start = 20;
        const uint32_t pet_total_count = 24;
        const uint32_t pet_drop_rate[pet_total_count] = {
            25600,
            25600,
            25600,
            25600,
            25600,
            25600,
            25600,
            25600,
            3200,
            3200,
            3200,
            3200,
            3200,
            3200,
            3200,
            3200,
            800,
            800,
            800,
            800,
            40,
            40,
            40,
            40,
        };

        int sum = 0;
        if (type == pgt_low_class) {
            sum = sum_low;
        } else {
            sum = sum_high;
        }

        auto rval = player_controller.begin_random(from);
        for (int index = 0; index < count; ++index) {
            int pos = player_controller.random_range(rval, sum);
            int value = 0;

            int start = 0;
            int end = pet_total_count;
            if (type == pgt_low_class) {
                end = legend_start;
            } else {
                start = rare_start;
            }

            for (int index = start; index < end; index++) {
                value += pet_drop_rate[index];
                int code = index + 1;
                if (pos < value) {
                    add_pet(from, code);
                    break;
                }
            }
        }

        player_controller.end_random(from, rval);
    }

    /// @brief
    /// Level up the pet. Magic powder is consumed.
    /// If the number of cards for level up is insufficient or the powder is insufficient, it fails.
    /// @param from
    /// Player who requested pet level up
    /// @param code
    /// pet code to level up
    int8_t petlvup(name from, uint16_t code) {
        require_auth(from);

        auto iter = find(from);
        auto &rows = get_pets(iter);
        int index = 0;
        for (index = 0; index < rows.size(); index++) {
            if (rows[index].code == code) {
                break;
            }
        }

        assert_true(index < rows.size(), "could not find pet");
        auto &pet = rows[index];
        
        int8_t knight = pet.knight;
        auto &petlv_rule = rpetlv_controller.get_table();
        auto lvrule = petlv_rule.find(pet.level + 1);
        assert_true(lvrule != petlv_rule.cend(), "could not find pet level rule");
        assert_true(pet.count >= lvrule->count, "not enough pet count");

        auto &pet_rule = rpet_controller.get_table();
        auto rule = pet_rule.find(code);
        assert_true(rule != pet_rule.cend(), "could not find pet rule");

        if (rule->grade == ig_normal) {
            assert_true(pet.level < kv_pet_normal_max_up, "already max up");
        } else if (rule->grade == ig_rare) {
            assert_true(pet.level < kv_pet_rare_max_up, "already max up");
        } else if (rule->grade == ig_unique) {
            assert_true(pet.level < kv_pet_unique_max_up, "already max up");
        } else if (rule->grade == ig_legendary) {
            assert_true(pet.level < kv_pet_legendary_max_up, "already max up");
        } else if (rule->grade == ig_ancient) {
            assert_true(pet.level < kv_pet_ancient_max_up, "already max up");
        }

        auto player = player_controller.get_player(from);
        assert_true(!player_controller.is_empty_player(player), "could not find player");

        int powder = 0;
        switch (rule->grade) {
            case ig_normal: powder = lvrule->powder1; break;
            case ig_rare: powder = lvrule->powder2; break;
            case ig_unique: powder = lvrule->powder3; break;
            case ig_legendary: powder = lvrule->powder4; break;
            case ig_ancient: powder = lvrule->powder5; break;
        }

        assert_true(powder <= player->powder, "not enough powder");
        if (powder > 0) {
            player_controller.decrease_powder(player, powder);
        }

        pets.modify(iter, self, [&](auto& pet){
            pet.rows[index].level++;
        });
        
        return knight;
    }

    /// @brief
    /// Assign the pet to the knight.
    /// Existing assigned pet is automatically detached.
    /// @param from
    /// Player who requested pet attach
    /// @param code
    /// pet code to attach
    /// @param code
    /// target knight for the pet
    void pattach(name from, uint16_t code, uint8_t knight) {
        require_auth(from);
        auto iter = find(from);
        pets.modify(iter, self, [&](auto& pet){
            bool exist = false;
            for (int index = 0; index < pet.rows.size(); index++) {
                if (pet.rows[index].knight == knight) {
                    pet.rows[index].knight = 0;
                }

                if (pet.rows[index].code == code) {
                    pet.rows[index].knight = knight;
                    exist = true;
                }
            }

            assert_true(exist, "can not pet");
        });
    }

    /// @brief
    /// Returns a controller that can CRUD the rule.
    /// @return
    /// Rule controller for pet rule
    rule_controller<rpet, rpet_table>& get_pet_rule() {
        return rpet_controller;
    }

    /// @brief
    /// Returns a controller that can CRUD the rule.
    /// @return
    /// Rule controller for pet level rule
    rule_controller<rpetlv, rpetlv_table>& get_pet_level_rule() {
        return rpetlv_controller;
    }

private:
    void add_stat(knight_stats &stat, stat_type type, int value) {
        switch (type) {
            case st_attack:
                stat.attack += value;
                break;
            case st_defense:
                stat.defense += value;
                break;
            case st_hp:
                stat.hp += value;
                break;
            case st_luck:
                stat.luck += value;
                break;
            default:
                assert_true(false, "invalid stat type");
        }
    }
};
