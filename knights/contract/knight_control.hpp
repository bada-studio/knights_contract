#pragma once


class knight_control_actions {
public:
    virtual void lvupknight(name from, uint8_t type) = 0;
    virtual void rebirth(name from, uint32_t season, uint32_t checksum, bool delay) = 0;
    virtual void setkntstage(name from, uint8_t stage) = 0;
    virtual void equip(name from, uint8_t to, uint32_t id) = 0;
    virtual void detach(name from, uint32_t id) = 0;
};


/*
 * base knight controller
 */
template<typename knight_table_name, 
         typename knight_table_const_iter_name, 
         typename player_name,
         typename player_table_const_iter_name,
         typename player_control_name,
         typename material_control_name,
         typename item_control_name,
         typename pet_control_name>
class knight_control_base : public control_base, 
                            public knight_control_actions {
protected:
    account_name self;
    knight_table_name knights;

    system_control &system_controller;
    player_control_name &player_controller;
    material_control_name &material_controller;
    item_control_name &item_controller;
    pet_control_name &pet_controller;

    std::vector<knightrow> empty_knightrows;

public:
    // constructor
    //-------------------------------------------------------------------------
    knight_control_base(account_name _self,
                   system_control &_system_controller,
                   player_control_name &_player_controller,
                   material_control_name &_material_controller,
                   item_control_name &_item_controller,
                   pet_control_name &_pet_controller)
            : self(_self)
            , knights(_self, _self)
            , system_controller(_system_controller)
            , player_controller(_player_controller)
            , material_controller(_material_controller)
            , item_controller(_item_controller)
            , pet_controller(_pet_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    knight_stats calculate_stat(name from, const knightrow &knight) {
        rknt_table rules(self, self);
        auto rule = rules.find(knight.type);
        assert_true(rule != rules.cend(), "no knight rule");

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

    const std::vector<knightrow>& get_knights(knight_table_const_iter_name iter) {
        if (iter != knights.cend()) {
            return iter->rows;
        }

        return empty_knightrows;
    }

    const knightrow& get_knight(knight_table_const_iter_name iter, uint8_t type) {
        for (int index = 0; index < iter->rows.size(); index++) {
            if (iter->rows[index].type == type) {
                return iter->rows[index];
            }
        }

        assert_true(false, "can not found knight");
        return iter->rows[0];
    }

    void new_free_knight(name from) {
        rknt_table rules(self, self);
        auto rule = rules.find(kt_knight);
        assert_true(rule != rules.cend(), "no knight rule");

        knightrow knight;
        knight.type = kt_knight;
        knight.level = 1;
        knight.attack = rule->attack;
        knight.defense = rule->defense;
        knight.hp = rule->hp;
        knight.luck = rule->luck;

        int count = 1;
        auto iter = knights.find(from);
        knights.emplace(self, [&](auto &target) {
            target.owner = from;
            target.rows.push_back(knight);
        });
    }

    knightrow new_knight(uint8_t type) {
        rknt_table rules(self, self);
        auto rule = rules.find(type);
        assert_true(rule != rules.cend(), "no knight rule");

        knightrow knight;
        knight.type = type;
        knight.level = 1;
        knight.attack = rule->attack;
        knight.defense = rule->defense;
        knight.hp = rule->hp;
        knight.luck = rule->luck;
        return knight;
    }

    // actions
    //-------------------------------------------------------------------------
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

        rkntlv_table rule_table(self, self);
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
    void rebirth(name from, uint32_t season, uint32_t checksum, bool delay) {
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        auto pvsi = system_controller.get_playervs(from);

        if (delay && USE_DEFERRED == 1) {
            require_auth(from);
            delay = system_controller.set_deferred(pvsi);

            if (do_rebirth(from, player, delay, pvsi)) {
                eosio::transaction out{};
                out.actions.emplace_back(
                    permission_level{ self, N(active) }, 
                    self, N(rebirth3i), 
                    std::make_tuple(from, season, checksum)
                );
                out.delay_sec = 1;
                out.send(system_controller.get_last_trx_hash(), self);
            }
        } else {
            if (USE_DEFERRED == 1) {
                require_auth(self);
            } else {
                require_auth(from);
            }

            do_rebirth(from, player, false, pvsi);
        }
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

        rstage_table rules(self, self);
        auto stagerule = rules.find(stage);
        assert_true(stagerule != rules.cend(), "no stage rule");

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

        ritem_table rule_table(self, self);
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

    void log_account(name from) {
        tklog_table table(self, self);
        auto iter = table.find(from);
        if (iter == table.cend()) {
            table.emplace(self, [&](auto &target) {
                target.owner = from;
                target.count = 1;
            });
        } else {
            table.modify(iter, self, [&](auto &target) {
                target.count++;
            });
        }
    }

    double get_bonus_floor_bonus(int floor, double avg_floor) {
        auto res_old = std::min(1000, floor) / 500.0;
        auto base_floor = avg_floor * 5;
        auto res_new = std::min((int)(base_floor), floor) / (base_floor * 0.5);

        uint32_t base_time = 48892800;
        uint32_t now = time_util::now_shifted();
        if (now < base_time) {
            return res_old;
        }

        uint32_t diff = now - base_time;
        uint32_t ease_base = time_util::day * 7;
        if (diff > ease_base) {
            return res_new;
        }

        double ease = (double)diff / (double)ease_base;
        return res_old + (res_new - res_old) * ease;
    }

    /// rebirth common logic
    bool do_rebirth(name from, player_table_const_iter_name player, bool only_check, playerv2_table::const_iterator pvsi) {
        system_controller.check_blacklist(from);
        system_controller.require_action_count(1);
        playerv2 variable = *pvsi;

        auto iter = knights.find(from);
        assert_true(iter != knights.cend(), "can not found knight");
        auto &rows = iter->rows;

        int total_kill_count = 0;
        int old_max_floor = player->maxfloor;
        int exp_mat_count = material_controller.get_current_inventory_size(from) + rows.size();
        int max_mat_count = material_controller.get_max_inventory_size(*player);
        assert_true(exp_mat_count <= max_mat_count, "insufficient inventory");

        rstage_table rules(self, self);
        auto stagerule = rules.find(player->current_stage);
        assert_true(stagerule != rules.cend(), "no stage rule");

        time current = time_util::now_shifted();
        int elapsed_sec = (int)(current - player->last_rebirth);
        
        set_rebirth_factor(player, variable, rows);

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

        int floor = (total_kill_count / 10) + 1;
        if (only_check) {
            if (floor < get_floor_for(ig_unique)) {
                only_check = false;
            } else {
                return true;
            }
        }

        knights.modify(iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                int type = target.rows[index].type;
                target.rows[index].kill_count += kill_counts[type];
            }
        });

        auto avg_floor = system_controller.get_global_avg_floor();
        auto gdr = system_controller.get_global_drop_factor(avg_floor);
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
        powder = (int)(powder * (1.0 + get_bonus_floor_bonus(floor, avg_floor)) * gdr);
        if (powder <= 0) {
            powder = 1;
        }

        auto rval = system_controller.begin_random(variable);
        uint16_t botties[kt_count] = {0, };
        for (int index = 1; index < kt_count; index++) {
            int pet_code = pet_controller.get_pet_for(from, index);

            if (kill_counts[index] > 0) {
                botties[index] = get_botties(*player, floor, lucks[index], kill_counts[index], *stagerule, rval, pet_code, gdr);
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

        variable.clear_deferred_time();
        if (rows.size() == kt_count-1 && floor > 10) {
            submit_floor(variable, old_max_floor, floor);
        }

        system_controller.end_random(variable, rval);
        system_controller.update_playerv(pvsi, variable);
        return only_check;
    }

    void set_rebirth_factor(player_table_const_iter_name player, playerv2 &variable, const std::vector<knightrow> &knights) {
        time current = time_util::now_shifted();
        double rebrith_factor = variable.rebrith_factor / 100.0;
        rebrith_factor = std::max(1.0, rebrith_factor);
        rebrith_factor = std::min(15.0, rebrith_factor);

        int life = 0;
        int old_total_kill;
        for (auto iter = knights.cbegin(); iter != knights.cend(); iter++) {
            auto &knight = *iter;
            int max_sec = calculate_max_alive_time(knight);
            life = std::max(life, max_sec);
            old_total_kill += knight.kill_count;
        }

        if (old_total_kill > 0) {
            uint32_t next = player->last_rebirth + (int)(kv_min_rebirth * rebrith_factor);
            assert_true(current >= next, "too short to get rebirth");
        }

        int threshold = std::min<int>(life / 4, 30 * time_util::min);
        int past = current - player->last_rebirth;
        if (past < threshold) {
            double rate = 1 + (threshold - past) / (double)threshold;
            rebrith_factor *= rate;
        } else {
            double rate = 1 + (past - threshold) / (double)threshold;
            rebrith_factor /= rate;
        }

        rebrith_factor = std::max(1.0, rebrith_factor);
        rebrith_factor = std::min(15.0, rebrith_factor);
        variable.rebrith_factor = (int)(rebrith_factor * 100);
    }

    int get_pet_for_knight(const std::vector<petrow>& rows, int knt) {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].knight == knt) {
                return rows[index].code;
            }
        }

        return 0;
    }

    void submit_floor(playerv2 &variable, int old_max_floor, int floor) {
        int new_count = 0;
        int new_floor = 0;

        if (variable.floor_submit == 0) {
            variable.floor_submit = 1;

            new_count++;
            new_floor = std::max(old_max_floor, floor);

        } else {
            if (floor > old_max_floor) {
                new_floor = floor - old_max_floor;
            }
        }

        if (new_floor > 0) {
            globalvar_table table(self, self);
            if (table.cbegin() == table.cend()) {
                table.emplace(self, [&](auto &target) {
                    target.id = 0;
                    target.floor_sum = new_floor;
                    target.floor_submit_count = new_count;
                });
            } else {
                table.modify(table.cbegin(), self, [&](auto &target) {
                    target.floor_sum += new_floor;
                    target.floor_submit_count += new_count;
                });
            }
        }
    }

    int get_botties(const player_name& from, int floor, int luck, int kill_count, const rstage& stagerule, random_val &rval, int pet_code, double gdr) {
        rmaterial_table mat_rules(self, self);
        double drop_rate = get_drop_rate_with_luck(stagerule.drop_rate, luck);
        int bonus_grade = pet_controller.get_pet_grade(pet_code);

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
            double dr = drop_rates[index] * (drop_rate / 100.0) * gdr;

            // same pet bonus
            int current_grade = get_field_material_grade(index + 1);
            if (bonus_grade == current_grade) {
                dr *= (kv_pet_attach_bonus / 100.0);
            }

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
};


/*
 * normal mode knight controller
 */
class knight_control : public knight_control_base<
    knight_table, 
    knight_table::const_iterator, 
    player,
    player_table::const_iterator,
    player_control,
    material_control,
    item_control,
    pet_control> {

private:
    kntskills_table skills;
    std::vector<kntskill> empty_kntskill;
    saleslog_control &saleslog_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    knight_control(account_name _self,
                   system_control &_system_controller,
                   player_control &_player_controller,
                   material_control &_material_controller,
                   item_control &_item_controller,
                   pet_control &_pet_controller,
                   saleslog_control &_saleslog_controller)
            : knight_control_base(
                _self,
                _system_controller,
                _player_controller,
                _material_controller,
                _item_controller,
                _pet_controller)
            , saleslog_controller(_saleslog_controller)                
            , skills(_self, _self) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    const std::vector<kntskill>& get_knight_skills(name from, int knt) {
        auto iter = skills.find(from);
        return get_knight_skills(iter, knt);
    }

    const std::vector<kntskill>& get_knight_skills(kntskills_table::const_iterator iter, int knt) {
        if (iter != skills.cend()) {
            return iter->cget_skills(knt);
        }

        return empty_kntskill;
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

        knightrow knight = new_knight(type);

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

        rkntprice_table prices(self, self);
        auto price_itr = prices.find(count);
        assert_true(price_itr != prices.cend(), "could not find price rule");

        // pay the cost
        asset price = price_itr->price;
        assert_true(quantity.amount == price.amount, "knight price does not match");

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::now_shifted();
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

        time current = time_util::now_shifted();
        players.modify(player, self, [&](auto& target) {
            target.last_rebirth = current;
        });
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
        rkntskills_table rules(self, self);
        const auto &rule = rules.begin()->get_rule(id);

        // required level check
        assert_true(rule.requiredlv <= knight.level, "not enough knight level");

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

private:
    int get_skill_count(const std::vector<kntskill> &skills) const {
        int res = 0;
        for (int index=0; index < skills.size(); index++) {
            res += skills[index].level;
        }
        return res;
    }
};
