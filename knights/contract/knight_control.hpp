#pragma once

class knight_control : public control_base {
private:
    account_name self;
    knight_table knights;
    kntskills_table skills;

    material_control &material_controller;
    item_control &item_controller;
    player_control &player_controller;
    pet_control &pet_controller;
    saleslog_control &saleslog_controller;
    rule_controller<rknt, rknt_table> knight_rule_controller;
    rule_controller<rkntlv, rkntlv_table> knight_level_rule_controller;
    rule_controller<rkntprice, rkntprice_table> knight_price_rule_controller;
    rule_controller<rkntskills, rkntskills_table> knight_skill_rule_controller;
    rule_controller<rstage, rstage_table> stage_rule_controller;
    std::vector<knightrow> empty_knightrows;
    std::vector<kntskill> empty_kntskill;

public:
    // constructor
    //-------------------------------------------------------------------------
    knight_control(account_name _self,
                   material_control &_material_controller,
                   item_control &_item_controller,
                   pet_control &_pet_controller,
                   player_control &_player_controller,
                   saleslog_control &_saleslog_controller)
            : self(_self)
            , knights(_self, _self)
            , skills(_self, _self)
            , knight_rule_controller(_self, N(knt))
            , knight_level_rule_controller(_self, N(kntlv))
            , knight_price_rule_controller(_self, N(kntprice))
            , knight_skill_rule_controller(_self, N(kntskills))
            , stage_rule_controller(_self, N(stage))
            , material_controller(_material_controller)
            , item_controller(_item_controller)
            , pet_controller(_pet_controller)
            , player_controller(_player_controller)
            , saleslog_controller(_saleslog_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    knight_stats calculate_stat(name from, const knightrow &knight) {
        auto rule = knight_rule_controller.get_table().find(knight.type);
        assert_true(rule != knight_rule_controller.get_table().cend(), "no knight rule");

        knight_stats res;
        res.attack = rule->attack + rule->gattack * (knight.level - 1);
        res.defense = rule->defense + rule->gdefense * (knight.level - 1);
        res.hp = rule->hp + rule->ghp * (knight.level - 1);
        res.luck = rule->luck;

        item_controller.apply_equip_stats(res, from, knight.type);
        pet_controller.apply_pet_stats(res, from, knight.type);
        return res;
    }

    void refresh_stat(name from, uint8_t type) {
        auto iter = knights.find(from);
        assert_true(iter != knights.cend(), "can not found knight");
        
        auto &knight = get_knight(iter, type);
        auto res = calculate_stat(from, knight);

        knights.modify(iter, self, [&](auto &target){
            for (int index = 0; index < target.rows.size(); index++) {
                if (target.rows[index].type == type) {
                    target.rows[index].attack = res.attack;
                    target.rows[index].defense = res.defense;
                    target.rows[index].hp = res.hp;
                    target.rows[index].luck = res.luck;
                    break;
                }
            }
        });
    }

    const std::vector<knightrow>& get_knights(name from) {
        auto iter = knights.find(from);
        return get_knights(iter);
    }

    const std::vector<kntskill>& get_knight_skills(name from, int knt) {
        auto iter = skills.find(from);
        return get_knight_skills(iter, knt);
    }

    int get_knight_max_level(name from) {
        auto &knights = get_knights(from);
        int level = 0;
        for (int index = 0; index < knights.size(); index++) {
            if (knights[index].level > level) {
                level = knights[index].level;
            }
        }

        return level;
    }

    const std::vector<knightrow>& get_knights(knight_table::const_iterator iter) {
        if (iter != knights.cend()) {
            return iter->rows;
        }

        return empty_knightrows;
    }

    const std::vector<kntskill>& get_knight_skills(kntskills_table::const_iterator iter, int knt) {
        if (iter != skills.cend()) {
            return iter->cget_skills(knt);
        }

        return empty_kntskill;
    }

    const knightrow& get_knight(knight_table::const_iterator iter, uint8_t type) {
        for (int index = 0; index < iter->rows.size(); index++) {
            if (iter->rows[index].type == type) {
                return iter->rows[index];
            }
        }

        assert_true(false, "can not found knight");
        return iter->rows[0];
    }

    // actions
    //-------------------------------------------------------------------------
    /// @brief
    /// hire new knight. player could pay the hire cost.
    /// @param from
    /// Player who requested hire action
    /// @param type
    /// knight who you want to hire
    void hireknight(name from, uint8_t type, const asset& quantity) {
        require_auth(from);
        assert_true(type > 0 && type < kt_count, "invalid knight type");

        auto rule = knight_rule_controller.get_table().find(type);
        assert_true(rule != knight_rule_controller.get_table().cend(), "no knight rule");

        knightrow knight;
        knight.type = type;
        knight.level = 1;
        knight.attack = rule->attack;
        knight.defense = rule->defense;
        knight.hp = rule->hp;
        knight.luck = rule->luck;

        int count = 1;
        auto iter = knights.find(from);
        if (iter == knights.cend()) {
            knights.emplace(self, [&](auto &target) {
                target.owner = from;
                target.rows.push_back(knight);
            });
        } else {
            bool found = false;
            knights.modify(iter, self, [&](auto &target) {
                for (int index = 0; index < target.rows.size(); index++) {
                    if (target.rows[index].type == type) {
                        found = true;
                        break;
                    }
                }
                
                target.rows.push_back(knight);
                count = target.rows.size();
            });

            assert_true(found == false, "you have already same knight");
        }

        auto price_itr = knight_price_rule_controller.get_table().find(count);
        assert_true(price_itr != knight_price_rule_controller.get_table().cend(), "could not find price rule");

        // pay the cost
        asset price = price_itr->price;
        assert_true(quantity.amount == price.amount, "knight price does not match");

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::getnow();
        blog.type = ct_knight;
        blog.pid = 0;
        blog.code = type;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = price;
        saleslog_controller.add_buylog(blog, from);

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "could not find player");

        time current = time_util::getnow();
        players.modify(player, self, [&](auto& target) {
            target.last_rebirth = current;
        });
    }

    /// @brief
    /// level up knight. it will decrease powder.
    /// @param from
    /// Player who requested level up action
    /// @param type
    /// knight who you want to level up
    void lvupknight(name from, uint8_t type) {
        require_auth(from);
        assert_true(type > 0 && type < kt_count, "invalid knight type");

        auto iter = knights.find(from);
        assert_true(iter != knights.cend(), "could not found knight");
        auto &knight = get_knight(iter, type);

        auto player = player_controller.get_player(from);
        assert_true(player_controller.is_empty_player(player) == false, "could not find player");

        uint64_t level = knight.level + 1;
        assert_true(level <= kv_max_knight_level, "already max level");

        auto &rule_table = knight_level_rule_controller.get_table();
        auto rule = rule_table.find(level);
        assert_true(rule != rule_table.cend(), "there is no level rule");

        int powder = rule->powder;
        assert_true(knight.kill_count >= rule->exp, "Insufficient exp");
        assert_true(powder <= player->powder, "Insufficient powder");

        if (powder > 0) {
            player_controller.decrease_powder(player, powder);
        }

        knights.modify(iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                if (target.rows[index].type == type) {
                    target.rows[index].level = level;
                    
                    auto stat = calculate_stat(from, knight);
                    target.rows[index].attack = stat.attack;
                    target.rows[index].defense = stat.defense;
                    target.rows[index].hp = stat.hp;
                    target.rows[index].luck = stat.luck;
                    break;
                }
            }
        });
    }

    /// @brief
    /// Rebirth all knights.
    /// @param from
    /// Player who requested rebirth
    /// @param checksum
    /// To prevent bots
    void rebirth(name from) {
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        do_rebirth(from, player);
    }
    
    /// @brief
    /// Change knight battle stage
    /// @param from
    /// Player who requested change stage
    /// @param knt
    /// Knight type
    /// @param stage
    /// Stage code
    void setkntstage(name from, uint8_t stage) {
        require_auth(from);

        auto stagerule = stage_rule_controller.get_table().find(stage);
        assert_true(stagerule != stage_rule_controller.get_table().cend(), "no stage rule");

        int minlv = stagerule->lvfrom;
        bool pass = false;
        
        auto iter = knights.find(from);
        assert_true(iter != knights.cend(), "could not found knight");
        auto &rows = iter->rows;

        for (auto iter = rows.cbegin(); iter != rows.cend(); iter++) {
            auto &knight = *iter;
            if (knight.level >= minlv) {
                pass = true;
                break;
            }
        }

        assert_true(pass, "no one exceed stage minmium level");
        auto &players = player_controller.get_players();
        auto player = players.find(from);

        players.modify(player, self, [&](auto& target) {
            target.current_stage = stage;
        });
    }

    /// @brief
    /// Equip item
    /// @param to
    /// Knight who you want to equip the item
    /// @param id
    /// Target item id
    void equip(name from, uint8_t to, uint32_t id) {
        require_auth(from);
        assert_true(to > 0 && to < kt_count, "invalid knight type");

        auto item_iter = item_controller.find(from);
        auto &rows = item_controller.get_items(item_iter);
        auto &item = item_controller.get_item(rows, id);
        assert_true(item.saleid == 0, "item is on sale");

        auto &rule_table = item_controller.get_ritem_rule().get_table();
        auto rule = rule_table.find(item.code);
        assert_true(rule != rule_table.cend(), "could not find rule");
        assert_true(is_valid_for((knight_type)to, (item_sub_type)rule->sub_type), "it's invalid knight to attach");

        auto knt_iter = knights.find(from);
        assert_true(knt_iter != knights.cend(), "could not found knight");
        auto &knight = get_knight(knt_iter, to);
        assert_true(rule->min_level <= knight.level, "not enough knight level to equip item");

        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].knight != to) {
                continue;
            }

            auto itr_rule = rule_table.find(rows[index].code);
            assert_true(itr_rule != rule_table.cend(), "could not find target item rule");
            if (itr_rule->type == rule->type) {
                item_controller.set_item_knight(item_iter, rows[index].id, 0);
                break;
            }
        }

        item_controller.set_item_knight(item_iter, id, to);
        refresh_stat(from, to);
    }

    /// @brief
    /// Detach item
    /// @param id
    /// Target item id
    void detach(name from, uint32_t id) {
        require_auth(from);

        auto iter = item_controller.find(from);
        auto &rows = item_controller.get_items(iter);
        auto &item = item_controller.get_item(rows, id);
        int8_t knight = item.knight;

        item_controller.set_item_knight(iter, id, 0);
        refresh_stat(from, knight);
    }

    /// @brief
    /// skill level up
    /// @param from
    /// account name
    /// @param knt
    /// target knight
    /// @param id
    /// skill id
    void skillup(name from, uint8_t knt, uint16_t id) {
        require_auth(from);

        auto knt_iter = knights.find(from);
        assert_true(knt_iter != knights.cend(), "could not found knight");
        auto &knight = get_knight(knt_iter, knt);
        int total_point = knight.level - 1;
        int current_point = 0;
        assert_true(total_point > 0, "no remain skill point");

        bool is_first_skill = ((id % 10) == 1);

        // remain point check
        assert_true(knight_skill_rule_controller.is_empty() == false, "empty rule");
        const auto &rule = knight_skill_rule_controller.get_table().begin()->get_rule(id);
        
        auto iter = skills.find(from);
        if (iter == skills.end()) {
            // check first skill
            assert_true(is_first_skill, "required previous skill first");
            
            // new skill
            skills.emplace(self, [&](auto &target) {
                kntskill skill;
                skill.set(id, 1);
                target.owner = from;
                auto &skills = target.get_skills(knt);
                skills.push_back(skill);
            });
        } else {
            auto &target = iter->cget_skills(knt);

            // remain point check
            current_point = get_skill_count(target);
            assert_true(total_point > current_point, "no remain skill point");

            bool find_lhs = false;
            for (int index = 0; index < target.size(); index++) {
                if (target[index].code == id-1) {
                    find_lhs = true;
                }

                // full upgrade check
                if (target[index].code == id) {
                    assert_true(target[index].level < rule.maxlevel, "already full level");
                    break;
                }
            }

            // left hand side skill check
            if (is_first_skill == false && find_lhs == false) {
                assert_true(false, "required previous skill first");
            }
            
            skills.modify(iter, self, [&](auto &target) {
                auto &tskill = target.get_skills(knt);
                bool found = false;
                for (int index = 0; index < tskill.size(); index++) {
                    // level up
                    if (tskill[index].code == id) {
                        tskill[index].level++;
                        found = true;
                        break;
                    }
                }

                // new skill
                if (found == false) {
                    kntskill skill;
                    skill.set(id, 1);
                    tskill.push_back(skill);
                }
            });
        }
    }

    /// @brief
    /// reset knight's skillset
    /// @param from
    /// account name
    /// @param knt
    /// target knight
    void skillreset(name from, uint8_t knt) {
        auto iter = skills.find(from);
        assert_true(iter != skills.end(), "can not found skill set");

        // clear stat
        skills.modify(iter, self, [&](auto &target) {
            auto &tskill = target.get_skills(knt);
            assert_true(tskill.size() > 0, "no skill to clear");
            tskill.clear();
        });

        // pay the price
        auto player = player_controller.get_player(from);
        assert_true(player_controller.is_empty_player(player) == false, "could not find player");
        player_controller.decrease_powder(player, kv_skill_reset_price);
    }

    rule_controller<rknt, rknt_table>& get_knight_rule_controller() {
        return knight_rule_controller;
    }

    rule_controller<rkntlv, rkntlv_table>& get_knight_level_rule_controller() {
        return knight_level_rule_controller;
    }

    rule_controller<rkntprice, rkntprice_table>& get_knight_price_rule_controller() {
        return knight_price_rule_controller;
    }

    rule_controller<rkntskills, rkntskills_table>& get_knight_skill_rule_controller() {
        return knight_skill_rule_controller;
    }

    rule_controller<rstage, rstage_table>& get_stage_rule_controller() {
        return stage_rule_controller;
    }

private:
    int calculate_max_alive_time(const knightrow &knight) {
        double damage_per_min = kv_enemy_attack;
        damage_per_min -= (double)kv_enemy_attack * knight.defense / (knight.defense + kv_defense_base);
        int alive_sec = (int)(60 * knight.hp / damage_per_min);
        return alive_sec;
    }

    double get_drop_rate_with_luck(int stage_drop_rate, int luck) {
        double addition = stage_drop_rate * (luck / (double)(luck + kv_luck_base));
        return stage_drop_rate + addition;
    }

    material_type get_rand_material_type(int value, const rstage& rule) {
        int sum = rule.nature_drop_rate;
        if (value < sum) {
            return mt_nature;
        }

        sum += rule.steel_drop_rate;
        if (value < sum) {
            return mt_iron;
        }

        sum += rule.bone_drop_rate;
        if (value < sum) {
            return mt_bone;
        }

        sum += rule.skin_drop_rate;
        if (value < sum) {
            return mt_skin;
        }

        return mt_mineral;
    }

    /// rebirth common logic
    void do_rebirth(name from, player_table::const_iterator player) {
        require_auth(from);
        player_controller.check_blacklist(from);
        player_controller.require_action_count(1);

        auto iter = knights.find(from);
        assert_true(iter != knights.cend(), "can not found knight");
        auto &rows = iter->rows;

        int old_total_kill;
        for (int index = 0; index < rows.size(); index++) {
            old_total_kill += rows[index].kill_count;
        }

        int total_kill_count = 0;
        auto &mats = material_controller.get_materials(from);
        
        int exp_mat_count = mats.size() + rows.size();
        int max_mat_count = material_controller.get_max_inventory_size(*player);
        assert_true(exp_mat_count <= max_mat_count, "insufficient inventory");

        auto stagerule = stage_rule_controller.get_table().find(player->current_stage);
        assert_true(stagerule != stage_rule_controller.get_table().cend(), "no stage rule");

        time current = time_util::getnow();
        int elapsed_sec = (int)(current - player->last_rebirth);
        if (old_total_kill > 0) {
            assert_true(elapsed_sec >= kv_min_rebirth, "too short to get rebirth");
        }

        int kill_counts[kt_count] = {0, };
        int lucks[kt_count] = {0, };

        for (auto iter = rows.cbegin(); iter != rows.cend(); iter++) {
            auto &knight = *iter;
            int max_sec = calculate_max_alive_time(knight);
            int play_sec = elapsed_sec;
            if (play_sec > max_sec) {
                play_sec = max_sec;
            }

            int current_kill_count = knight.attack * play_sec / 60 / kv_enemy_hp;
            if (current_kill_count == 0) {
                current_kill_count = 1;
            }

            kill_counts[knight.type] = current_kill_count;
            lucks[knight.type] = knight.luck;
            total_kill_count += current_kill_count;
        }

        knights.modify(iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                int type = target.rows[index].type;
                target.rows[index].kill_count += kill_counts[type];
            }
        });

        int floor = (total_kill_count / 10) + 1;
        double powder = 0;
        for (int index = 1; index < kt_count; index++) {
            if (kill_counts[index] == 0) {
                continue;
            }

            double current_powder = kill_counts[index] / (double)kv_kill_powder_rate;
            double scaler = 1.0 + (lucks[index] / 1000.0);
            if (scaler > 8) { // barrier
                scaler = 8;
            }

            powder += current_powder * scaler;
        }
        
        // get high floor bonus #23
        powder = (int)(powder * (1.0 + (std::min(1000, floor) / 500.0)));
        if (powder <= 0) {
            powder = 1;
        }

        auto rval = player_controller.begin_random(from, r4_rebirth, 0);

        uint16_t botties[kt_count] = {0, };
        for (int index = 1; index < kt_count; index++) {
            if (kill_counts[index] > 0) {
                botties[index] = get_botties(*player, floor, lucks[index], kill_counts[index], *stagerule, rval);
            }
        }

        material_controller.add_materials(from, botties);
        auto &players = player_controller.get_players();
        players.modify(player, self, [&](auto& target) {
            target.last_rebirth = current;
            target.powder += powder;
            if (target.maxfloor < floor) {
                target.maxfloor = floor;
            }
        });

        player_controller.end_random(from, rval, r4_rebirth, 0);
    }

    int get_botties(const player& from, int floor, int luck, int kill_count, const rstage& stagerule, random_val &rval) {
        auto &mat_rules = material_controller.get_rmaterial_rule();
        double drop_rate = get_drop_rate_with_luck(stagerule.drop_rate, luck);

        // add floor bonus drop rate
        double floor_drop_bonus = kv_floor_bonus_1000 * (floor / 1000.0);
        drop_rate += floor_drop_bonus;

        if (drop_rate > 100.0) {
            drop_rate = 100.0;
        }
        
        int best = 0;
        int drscale = 1000000000;
        int rand_value = rval.range(drscale);

        // fix #16 reported by Jinhyeon Hong
        int start_index = drop_rates_length - 1;
        if (floor < get_floor_for(ig_unique)) {
            start_index = 6;
        } else if (floor < get_floor_for(ig_legendary)) {
            start_index = 8;
        } else if (floor < get_floor_for(ig_ancient)) {
            start_index = 9;
        }

        for (int index = start_index; index >= 1; --index) {
            double dr = drop_rates[index] * (drop_rate / 100.0);
            double mdr = 1.0 - pow(1.0 - dr, kill_count);
            if (rand_value < int(mdr * drscale)) {
                best = index;
                break;
            }
        }

        int mtvalue = rval.range(100);
        material_type type = get_rand_material_type(mtvalue, stagerule);

        int code = (type - 1) * 20 + (best + 1);
        return code;
    }

    int get_floor_for(item_grade grade) {
        int data = kv_required_floor_for_material;
        if (grade == ig_unique) {
            return (data & 0xF) * 100;
        } else if (grade == ig_legendary) {
            return ((data >> 4) & 0xF) * 100;
        } else if (grade == ig_ancient) {
            return ((data >> 8) & 0xF) * 100;
        }

        return 0;
    }

    bool is_valid_for(knight_type kt, item_sub_type ist) {
        if (ist == ist_axe && kt != kt_knight) {
            return false;
        }
        if (ist == ist_bow && kt != kt_archer) {
            return false;
        }
        if (ist == ist_staff && kt != kt_mage) {
            return false;
        }
        return true;
    }

    int get_skill_count(const std::vector<kntskill> &skills) const {
        int res = 0;
        for (int index=0; index < skills.size(); index++) {
            res += skills[index].level;
        }
        return res;
    }
};
