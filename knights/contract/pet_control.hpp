#pragma once

class pet_control : public drop_control_base {
private:
    account_name self;

    player_control &player_controller;
    material_control &material_controller;
    saleslog_control &saleslog_controller;

    std::vector<petrow> empty_petrows;

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    pet_control(account_name _self,
                player_control &_player_controller,
                material_control &_material_controller)
            : self(_self)
            , player_controller(_player_controller)
            , material_controller(_material_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    const std::vector<petrow>& get_pets(name from) {
        pet_table pets(self, self);
        auto iter = pets.find(from);
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

        pet_table pets(self, self);
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
        rpet_table pet_rule(self, self);
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

    int get_pet_grade(int code) {
        if (code <= 0) {
            return ig_none;
        }

        if (code <= 8) {
            return ig_normal;
        }
        if (code <= 16) {
            return ig_rare;
        }
        if (code <= 20) {
            return ig_unique;
        }
        if (code <= 24) {
            return ig_legendary;
        }
        if (code <= 26) {
            return ig_ancient;
        }
        
        return ig_none;
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
    /// @param checksum
    /// To prevent bots
    void petgacha(name from, uint16_t type, uint8_t count, uint32_t checksum, bool delay, bool frompay) {
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "could not find player");
        auto pvsi = player_controller.get_playervs(from);

        if (delay && USE_DEFERRED == 1) {
            require_auth(from);
            delay = player_controller.set_deferred(pvsi);

            if (do_petgacha(player, type, count, delay, pvsi)) {
                eosio::transaction out{};
                out.actions.emplace_back(
                    permission_level{ self, N(active) }, 
                    self, N(petgacha2i), 
                    std::make_tuple(from, type, count, checksum)
                );
                out.delay_sec = 1;
                out.send(player_controller.get_last_trx_hash(), self);
            }
        } else {
            if (USE_DEFERRED == 1) {
                require_auth(self);
            } else {
                require_auth(from);
            }

            do_petgacha(player, type, count, false, pvsi);
        }
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

        pet_table pets(self, self);
        auto iter = pets.find(from);
        assert_true(iter != pets.cend(), "could not find pet");
        auto &rows = iter->rows;

        int index = 0;
        for (index = 0; index < rows.size(); index++) {
            if (rows[index].code == code) {
                break;
            }
        }

        assert_true(index < rows.size(), "could not find pet");
        auto &pet = rows[index];
        
        int8_t knight = pet.knight;
        rpetlv_table petlv_rule(self, self);
        auto lvrule = petlv_rule.find(pet.level + 1);
        assert_true(lvrule != petlv_rule.cend(), "could not find pet level rule");
        assert_true(pet.count >= lvrule->count, "not enough pet count");

        rpet_table pet_rule(self, self);
        auto rule = pet_rule.find(code);
        assert_true(rule != pet_rule.cend(), "could not find pet rule");

        auto max_level = get_max_pet_level(rule->grade);
        assert_true(pet.level < max_level, "already max up");

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
        assert_true(is_pet_free(from, code), "the pet is on expedition or resting");

        pet_table pets(self, self);
        auto iter = pets.find(from);
        assert_true(iter != pets.cend(), "can not found pet");
        
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

            assert_true(exist, "can not found pet");
        });
    }

    void pexpstart(name from, uint16_t code, int knight_max_level) {
        require_auth(from);

        pet_table pets(self, self);
        petexp_table petexps(self, self);
        auto exp_iter = petexps.find(from);
        auto pet_iter = pets.find(from);
        assert_true(pet_iter != pets.cend(), "no pets");
        auto &pet_rows = pet_iter->rows;

        // check knight
        bool found = false;
        for (int index = 0; index < pet_rows.size(); index++) {
            auto &pet = pet_rows[index];
            if (pet.code != code) {
                continue;
            }
            assert_true (pet.knight == 0, "already fight with knight");
            found = true;
            break;
        }

        assert_true(found, "can not found pet");
        rpet_table pet_rule(self, self);
        auto rule = pet_rule.find(code);
        assert_true(rule != pet_rule.cend(), "could not find pet rule");
        auto duration = get_pet_exp_duration(rule->grade);
        auto current = time_util::now_shifted();
        int max_slots = get_pex_slots(knight_max_level);

        petexprow row;
        row.code = code;
        row.start = current;
        row.end = current + duration;

        if (exp_iter == petexps.cend()) {
            petexps.emplace(self, [&](auto& target){
                target.owner = from;
                target.rows.push_back(row);
            });
        } else {
            std::vector<petexprow> updated;
            auto &rows = exp_iter->rows;

            int count = 0;
            for (int index = 0; index < rows.size(); index++) {
                auto &pet = rows[index];
                if (current >= pet.end && pet.isback) {
                    continue;
                }

                if (pet.isback == false) {
                    count++;
                }

                updated.push_back(pet);
                assert_true(pet.code != code, "already in expedition");
            }

            assert_true(count < max_slots, "exceed max slots");
            updated.push_back(row);

            petexps.modify(exp_iter, self, [&](auto& target){
                target.owner = from;
                target.rows = updated;
            });
        }
    }

    void pexpreturn(name from, uint16_t code, uint32_t checksum, bool delay, bool frompay) {
        auto pvsi = player_controller.get_playervs(from);

        if (delay && USE_DEFERRED == 1) {
            require_auth(from);
            delay = player_controller.set_deferred(pvsi);

            if (do_pexpreturn(from, code, delay, pvsi)) {
                eosio::transaction out{};
                out.actions.emplace_back(
                    permission_level{ self, N(active) }, 
                    self, N(pexpreturn2i), 
                    std::make_tuple(from, code, checksum)
                );
                out.delay_sec = 1;
                out.send(player_controller.get_last_trx_hash(), self);
            }
        } else {
            if (USE_DEFERRED == 1) {
                require_auth(self);
            } else {
                require_auth(from);
            }

            do_pexpreturn(from, code, false, pvsi);
        }
    }

    bool do_pexpreturn(name from, uint16_t code, bool only_check, playerv2_table::const_iterator pvsi) {
        player_controller.require_action_count(1);
        auto gdr = player_controller.get_global_drop_factor();

        petexp_table petexps(self, self);
        auto exp_iter = petexps.find(from);
        assert_true(exp_iter != petexps.cend(), "could not find pet expedition data");

        rpet_table pet_rule(self, self);
        auto rule = pet_rule.find(code);
        assert_true(rule != pet_rule.cend(), "could not find pet rule");
        auto duration = get_pet_exp_duration(rule->grade);
        auto current = time_util::now_shifted();
        if (only_check && rule->grade < pg_unique) {
            only_check = false;
        }

        bool found = false;
        petexps.modify(exp_iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                auto &pet = target.rows[index];
                if (pet.code != code) {
                    continue;
                }

                assert_true(pet.isback == false, "already return");
                assert_true(pet.end < current, "too early return");
                if (only_check == false) {
                    pet.isback = true;
                    pet.end = current + duration;
                }
                found = true;
                break;
            }
        });

        assert_true(found, "can not found pet exp data");
        auto &pets = get_pets(from);
        found = false;
        int32_t level = 1;
        for (int index = 0; index < pets.size(); index++) {
            auto &pet = pets[index];
            if (pet.code == code) {
                level = pet.level;
                found = true;
                break;
            }
        }

        assert_true(found, "can not found pet data");
        rpetexp_table exp_rules(self, self);
        auto exp_rule = exp_rules.find(level);
        assert_true(exp_rule != exp_rules.cend(), "could not find pet rule");

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "could not find player");

        // check inventory size;
        auto &mats = material_controller.get_materials(from);
        int exp_mat_count = mats.size() + 1;
        int max_mat_count = material_controller.get_max_inventory_size(*player);
        assert_true(exp_mat_count <= max_mat_count, "insufficient inventory");

        if (only_check) {
            return true;
        }

        // calculate drop magic water
        int mw = exp_rule->get_mw(rule->grade);
        mw = mw * duration / time_util::day;

        auto variable = *pvsi;
        auto rval = player_controller.begin_random(variable);
        int range = (int)rval.range(21) - 10;
        mw += mw * range / 100;
        mw = std::max(0, mw);
        mw = std::min(10000, mw);
        
        players.modify(player, self, [&](auto& target) {
            target.powder += mw;
        });

        // determin drop material grade
        int bottie_grade = std::max(1, rule->grade - 1);
        int value = rval.range(100);
        if (value < (int)(exp_rule->get_drop_rate(rule->grade) * gdr)) {
            bottie_grade++;
        }

        if (rule->grade == 1) {
            bottie_grade = 1;
        }

        // determin material
        uint16_t bottie = get_bottie(*player, bottie_grade, rval);
        assert_true(bottie != 0, "invalid material drop");
        material_controller.add_material(from, bottie);

        player_controller.end_random(variable, rval);
        variable.clear_deferred_time();
        player_controller.update_playerv(pvsi, variable);
        return only_check;
    }

    bool is_pet_free(name from, int16_t code) {
        petexp_table petexps(self, self);
        auto exp_iter = petexps.find(from);
        if(exp_iter == petexps.cend()) {
            return true;
        }

        auto &rows = exp_iter->rows;
        auto current = time_util::now_shifted();

        for (int index = 0; index < rows.size(); index++) {
            auto &pet = rows[index];
            if (pet.code != code) {
                continue;
            }

            if (current < pet.end || pet.isback == false) {
                return false;
            }
            break;
        }

        return true;
    }

private:
    bool do_petgacha(player_table::const_iterator player, uint16_t type, uint8_t count, bool only_check, playerv2_table::const_iterator pvsi) {
        name from = player->owner;
        player_controller.require_action_count(1);

        assert_true(type > 0 && type < pgt_count, "invalid gacha type");
        assert_true(count > 0 && count < 10, "invalid count");

        if (only_check && type == pgt_low_class) {
            only_check = false;
        }

        int powder;
        if (type == pgt_low_class) {
            powder = kv_pet_gacha_low_price;
        } else {
            powder = kv_pet_gacha_high_price;
        }

        powder *= count;
        assert_true(powder <= player->powder, "not enough powder");
        if (powder > 0) {
            player_controller.decrease_powder(player, powder, only_check);
        }

        if (only_check) {
            return true;
        }

        //@ warning for the performance issue, drop rates are hard coded here,
        // be careful for the data sync issue with pet rule.
        const uint32_t sum_low = 233600;
        const uint32_t sum_high = 28968;
        const uint32_t rare_start = 8;
        const uint32_t legend_start = 20;
        const uint32_t pet_total_count = 26;
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
            4,
            4,
        };

        int sum = 0;
        if (type == pgt_low_class) {
            sum = sum_low;
        } else {
            sum = sum_high;
        }

        auto variable = *pvsi;
        auto rval = player_controller.begin_random(variable);
        for (int index = 0; index < count; ++index) {
            int pos = rval.range(sum);
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

        player_controller.end_random(variable, rval);
        variable.clear_deferred_time();
        player_controller.update_playerv(pvsi, variable);
        return only_check;
    }

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

    int32_t get_pet_exp_duration(int grade) {
        return ((kv_pet_exp_duration >> (grade - 1) * 4) & 0xF) * 6 * time_util::hour;
    }

    int32_t get_pex_slots(int level) {
        if (level < ((kv_pet_exp_require_level >> 4) & 0xf)) {
            return 1;
        }

        if (level < ((kv_pet_exp_require_level >> 8) & 0xf)) {
            return 2;
        }

        if (level < ((kv_pet_exp_require_level >> 12) & 0xf)) {
            return 3;
        }

        return 4;
    }

    int32_t get_max_pet_level(int grade) {
        return (kv_pet_max_up >> ((grade - 1) * 4)) & 0xf;
    }
};
