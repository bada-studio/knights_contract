#pragma once

class item_control : public control_base {
private:
    account_name self;
    item_table items;

    rule_controller<ritem, ritem_table> item_rule_controller;
    rule_controller<ritemlv, ritemlv_table> itemlv_rule_controller;
    rule_controller<ritemset, ritemset_table> itemset_rule_controller;
    material_control &material_controller;
    player_control &player_controller;
    saleslog_control &saleslog_controller;

    std::vector<itemrow> empty_itemrows;
    itemrow empty_itemrow;

public:
    // constructor
    //-------------------------------------------------------------------------
    item_control(account_name _self,
                 material_control &_material_controller,
                 player_control &_player_controller,
                 saleslog_control &_saleslog_controller)
            : self(_self)
            , items(_self, _self)
            , item_rule_controller(_self, N(item))
            , itemlv_rule_controller(_self, N(itemlv))
            , itemset_rule_controller(_self, N(itemset))
            , material_controller(_material_controller)
            , player_controller(_player_controller) 
            , saleslog_controller(_saleslog_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    int get_max_inventory_size(const player& player) {
        int size = kv_item_inventory_size;
        int upgrade = player.item_ivn_up;
        if (upgrade > kv_max_item_inventory_up) {
            upgrade = kv_max_item_inventory_up;
        }

        size += upgrade * kv_bonus_size_for_inventory_up;
        return size;
    }

    void apply_equip_stats(knight_stats &stat, name from, uint64_t knight) {
        auto &rule_table = item_rule_controller.get_table();
        auto &lvrules = itemlv_rule_controller.get_table();

        auto &rows = get_items(from);
        int32_t setid = 0;
        int32_t setcount = 0;
        int16_t codes[3] = {0, };
        int32_t bonuses[3] = {0, };
        int item_count = 0;

        for (int index = 0; index < rows.size(); index++) {
            auto &item = rows[index];
            if (item.knight != knight) {
                continue;
            }

            auto rule = rule_table.find(item.code);
            assert_true(rule != rule_table.cend(), "could not find rule");
            if (setid == 0) {
                setid = rule->setid;
            }

            if (setid != 0 && setid == rule->setid) {
                setcount++;
            }

            uint32_t rate1 = item.dna & 0xFF;
            uint32_t rate2 = (item.dna >> 8) & 0xFF;
            uint32_t rate3 = (item.dna >> 16) & 0xFF;
            uint32_t reveal2 = (item.dna >> 24) & 0x2;
            uint32_t reveal3 = (item.dna >> 24) & 0x4;

            auto lvrule = lvrules.find(item.level);
            assert_true(lvrule != lvrules.cend(), "could not find level rule");

            uint32_t bonus = lvrules.find(item.level)->bonus;
            bonuses[item_count] = bonus;
            codes[item_count] = item.code;

            uint32_t stat1 = (uint32_t)(rule->stat1 + get_variation_value(rule->stat1_rand_range, rate1));
            stat1 = apply_bonus_stat(stat1, bonus);
            add_stat(stat, (stat_type)rule->stat1_type, stat1);

            if (reveal2 > 0) {
                uint32_t stat2 = (uint32_t)(rule->stat2 + get_variation_value(rule->stat2_rand_range, rate2));
                stat2 = apply_bonus_stat(stat2, bonus);
                add_stat(stat, (stat_type)rule->stat2_type, stat2);
            }

            if (reveal3 > 0) {
                uint32_t stat3 = (uint32_t)(rule->stat3 + get_variation_value(rule->stat3_rand_range, rate3));
                stat3 = apply_bonus_stat(stat3, bonus);
                add_stat(stat, (stat_type) rule->stat3_type, stat3);
            }

            item_count++;
        }

        // apply set item stat
        if (setid > 0 && setcount == 3) {
            auto &table = get_ritemset_rule().get_table();
            auto iter = table.find(setid);
            if (iter == table.cend()) {
                return;
            }

            for (int index = 0; index < 3; index++) {
                auto rule = iter->get_element(codes[index]);
                assert_true(rule.type > 0, "can not found set rule");

                uint32_t current_stat = rule.stat;
                current_stat = apply_bonus_stat(current_stat, bonuses[index]);
                add_stat(stat, (stat_type)rule.type, current_stat);
            }
        }
    }

    void add_item(name from, int code, int dna, int level, int exp) {
        itemrow row;
        row.dna = dna;
        row.code = code;
        row.level = level;
        row.exp = exp;

        auto iter = items.find(from);
        if (iter == items.cend()) {
            items.emplace(self, [&](auto& item){
                row.id = 1;
                item.owner = from;
                item.last_id = row.id;
                item.rows.push_back(row);
            });
        } else {
            items.modify(iter, self, [&](auto& item){
                row.id = item.last_id + 1;
                item.owner = from;
                item.last_id = row.id;
                item.rows.push_back(row);
            });
        }
    }

    item_table::const_iterator find(name from) {
        return items.find(from);
    }

    const std::vector<itemrow>& get_items(name from) {
        auto iter = items.find(from);
        return get_items(iter);
    }

    const std::vector<itemrow>& get_items(item_table::const_iterator iter) {
        if (iter != items.cend()) {
            return iter->rows;
        }

        return empty_itemrows;
    }
    
    const itemrow& get_item(const std::vector<itemrow> &rows, int id) {
        // binary search
        int left = 0;
        int right = rows.size() - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (rows[mid].id < id) {
                left = mid + 1;
            } else if (id < rows[mid].id) {
                right = mid - 1;
            } else {
                return rows[mid];
            }
        }
        
        assert_true(false, "can not found item");
        return empty_itemrow;
    }

    void make_item_forsale(name from, uint64_t itemid, uint64_t saleid) {
        auto iter = items.find(from);
        assert_true(iter != items.end(), "could not find item");

        bool found = false;
        items.modify(iter, self, [&](auto& item){
            for (int index = 0; index < item.rows.size(); index++) {
                if (item.rows[index].id == itemid && item.rows[index].knight == 0) {
                    item.rows[index].saleid = saleid;
                    found = true;
                    break;
                }
            }
        });

        assert_true(found, "could not found item");
    }

    void cancel_sale(name from, uint64_t saleid) {
        auto iter = items.find(from);
        assert_true(iter != items.end(), "could not find item");

        bool found = false;
        items.modify(iter, self, [&](auto& item){
            for (int index = 0; index < item.rows.size(); index++) {
                if (item.rows[index].saleid == saleid) {
                    item.rows[index].saleid = 0;
                    found = true;
                    break;
                }
            }
        });

        assert_true(found, "could not found item");
    }

    void remove_saleitem(name from, uint64_t saleid) {
        auto iter = items.find(from);
        assert_true(iter != items.end(), "could not find item");

        int found = -1;
        items.modify(iter, self, [&](auto& item){
            for (int index = 0; index < item.rows.size(); index++) {
                if (item.rows[index].saleid == saleid) {
                    found = index;
                    break;
                }
            }

            if (found >= 0) {
                item.rows.erase(item.rows.begin() + found);
            }
        });

        assert_true(found >= 0, "could not found item");
    }

    void new_item_from_market(name from, uint16_t code, uint32_t dna, uint8_t level, uint8_t exp) {
        add_item(from, code, dna, level, exp);
    }

    uint32_t remove_items(name from, const std::vector<uint32_t> &item_ids) {
        auto &item_rule = item_rule_controller.get_table();
        auto iter = items.find(from);
        assert_true(iter != items.cend(), "could not found item");

        uint32_t powder = 0;
        int found = false;
        items.modify(iter, self, [&](auto& item){
            for (int index = 0; index < item_ids.size(); ++index) {
                found = false;
                int id = item_ids[index];

                // binary search
                auto &rows = item.rows;
                int left = 0;
                int right = rows.size() - 1;
                while (left <= right) {
                    int mid = left + (right - left) / 2;
                    if (rows[mid].id < id) {
                        left = mid + 1;
                    } else if (id < rows[mid].id) {
                        right = mid - 1;
                    } else {
                        auto &target = item.rows[mid];
                        // it is on sale or equipped
                        assert_true(target.saleid == 0, "can not remove item on sale");
                        assert_true(target.knight == 0, "can not remove equipped item");

                        // find powder rule
                        auto rule = item_rule.find(target.code);
                        assert_true(rule != item_rule.cend(), "can not found rule");

                        int count = target.exp + 1;
                        count = std::min(count, 16);
                        powder += rule->powder * count;
                        item.rows.erase(item.rows.begin() + mid);
                        found = true;
                        break;
                    }
                }

                if (found == false) {
                    break;
                }
            }
        });

        assert_true(found, "could not found item");
        return powder;
    }

    void set_item_knight(item_table::const_iterator iter, int32_t id, uint8_t knight) {
        bool found = false;
        items.modify(iter, self, [&](auto& item){
            for (int index = 0; index < item.rows.size(); index++) {
                if (item.rows[index].id == id) {
                    item.rows[index].knight = knight;
                    found = true;
                    break;
                }
            }
        });

        assert_true(found, "could not found item");
    }

    int calculate_item_score(uint16_t code, uint32_t dna) {
        auto &rule_table = item_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "could not find rule");

        uint32_t rate1 = dna & 0xFF; 
        uint32_t rate2 = (dna >> 8) & 0xFF; 
        uint32_t rate3 = (dna >> 16) & 0xFF;
        uint32_t reveal2 = (dna >> 24) & 0x2; 
        uint32_t reveal3 = (dna >> 24) & 0x4; 
        int stat1 = rule->stat1_rand_range + get_variation_value(rule->stat1_rand_range, (int)rate1);
        int stat2 = 0;
        if (reveal2 > 0) {
            stat2 = rule->stat2 + get_variation_value(rule->stat2_rand_range, (int)rate2);
        }
        int stat3 = 0;
        if (reveal3 > 0) {
            stat3 = rule->stat3 + get_variation_value(rule->stat3_rand_range, (int) rate3);
        }

        int stat1Max = rule->stat1_rand_range * 2;
        int stat2Max = rule->stat2 + rule->stat2_rand_range;
        int stat3Max = rule->stat3 + rule->stat3_rand_range;
        return (stat1 + stat2 + stat3) * 100 / (stat1Max + stat2Max + stat3Max);
    }

    // actions
    //-------------------------------------------------------------------------
    /// @brief
    /// Craft item
    /// @param from
    /// Player who requested craft item
    /// @param code
    /// item code which you want to craft
    /// @param mat_ids
    /// material ids for item, this materials will be deleted.
    void craft(name from, uint16_t code, const std::vector<uint32_t> &mat_ids) {
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");

        do_craft(player, code, mat_ids);
    }

    /// @brief
    /// Remove item
    /// @param from
    /// Player who requested remove item action
    /// @param item_ids
    /// items which are you want to remove
    void remove(name from, const std::vector<uint32_t> &item_ids) {
        require_auth(from);

        int powder = remove_items(from, item_ids);

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "can not found player");

        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }

    /// @brief
    /// Merge items
    /// @param from
    /// Player who requested merge item action
    /// @param id
    /// Target item
    /// @param ingredient
    /// Ingredient item ids. These items will be removed.
    void itemmerge(name from, uint32_t id, const std::vector<uint32_t> &ingredient) {
        require_auth(from);

        auto iter = items.find(from);
        auto &rows = get_items(iter);
        auto &item = get_item(rows, id);
        assert_true(item.saleid == 0, "item is on sale");

        auto &rtable = itemlv_rule_controller.get_table();
        auto &last = *--rtable.cend();
        assert_true(item.exp < last.count, "already full exp");

        int exp = 0;
        for (auto iter = ingredient.cbegin(); iter != ingredient.cend(); ++iter) {
            auto mat = get_item(rows, *iter);
            assert_true(mat.code == item.code, "invalid ingredient");
            exp += (mat.exp + 1);
        }

        remove_items(from, ingredient);
        items.modify(iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                if (target.rows[index].id != id) {
                    continue;
                }

                if (exp + target.rows[index].exp < 128) {
                    target.rows[index].exp += exp;
                } else {
                    target.rows[index].exp = 127;
                }
                break;
            }
        });
    }

    /// @brief
    /// Level up item
    /// @param from
    /// Player who requested level up action
    /// @param id
    /// Target item
    int8_t itemlvup(name from, uint64_t id) {
        require_auth(from);
        player_controller.require_action_count(1);

        auto iter = items.find(from);
        auto &rows = get_items(iter);
        auto &item = get_item(rows, id);
        assert_true(item.saleid == 0, "item is on sale");
        int8_t knight = item.knight;

        auto &lvtable = itemlv_rule_controller.get_table();
        auto lvrulec = lvtable.find(item.level);
        auto lvrule = lvtable.find(item.level + 1);
        assert_true(lvrulec != lvtable.cend(), "can not found next level rule");
        assert_true(lvrule != lvtable.cend(), "can not found next level rule");
        assert_true(item.exp >= lvrule->count, "insufficient item exp");
        auto required = lvrule->count - lvrulec->count;

        auto &rtable = item_rule_controller.get_table();
        auto rule = rtable.find(item.code);
        assert_true(rule != rtable.end(), "no item rule");

        uint32_t powder = 0;
        switch (rule->grade) {
            case ig_normal: powder = lvrule->powder1; break;
            case ig_rare: powder = lvrule->powder2; break;
            case ig_unique: powder = lvrule->powder3; break;
            case ig_legendary: powder = lvrule->powder4; break;
            case ig_ancient: powder = lvrule->powder5; break;
            case ig_chaos: powder = lvrule->powder6; break;
        }

        auto player = player_controller.get_player(from);
        assert_true(powder <= player->powder, "not enough powder");

        // level up success
        bool success = true;
        if (lvrule->rate < 10000) {
            auto rval = player_controller.begin_random(from, r4_craft, rule->grade);
            success = (rval.range(10000) < lvrule->rate);
            player_controller.end_random(from, rval, r4_craft, rule->grade);
        }

        if (!success) {
            powder /= 2;
        }

        player_controller.decrease_powder(player, powder);

        items.modify(iter, self, [&](auto& target) {
            for (int index = 0; index < target.rows.size(); index++) {
                if (target.rows[index].id != id) {
                    continue;
                }
                
                if (success) {
                    target.rows[index].level++;
                } else {
                    target.rows[index].exp -= required;
                }

                break;
            }
        });

        return knight;
    }

    rule_controller<ritem, ritem_table>& get_ritem_rule() {
        return item_rule_controller;
    }

    rule_controller<ritemlv, ritemlv_table>& get_ritemlv_rule() {
        return itemlv_rule_controller;
    }

    rule_controller<ritemset, ritemset_table>& get_ritemset_rule() {
        return itemset_rule_controller;
    }

private:
    void do_craft(player_table::const_iterator player, uint16_t code, const std::vector<uint32_t> &mat_ids) {
        name from = player->owner;
        require_auth(from);
        player_controller.require_action_count(1);

        auto &rule_table = item_rule_controller.get_table();
        auto recipe = rule_table.find(code);
        assert_true(recipe != rule_table.cend(), "could not find recipe");

        int inven_size = get_max_inventory_size(*player);
        assert_true(get_items(from).size() < inven_size, "full inventory");

        int mat1_count = recipe->mat1_count;
        int mat2_count = recipe->mat2_count;
        int mat3_count = recipe->mat3_count;
        int mat4_count = recipe->mat4_count;

        auto &mats = material_controller.get_materials(from);

        for (int index = 0; index < mat_ids.size(); index++) {
            auto &mat =  material_controller.get_material(mats, mat_ids[index]);
            assert_true(mat.saleid == 0, "material is on sale");

            if (mat.code == recipe->mat1_code) {
                mat1_count--;
            } else if (mat.code == recipe->mat2_code) {
                mat2_count--;
            } else if (mat.code == recipe->mat3_code) {
                mat3_count--;
            } else if (mat.code == recipe->mat4_code) {
                mat4_count--;
            } else {
                assert_true(false, "invalid recipe material");
            }
        }

        assert_true(mat1_count == 0 &&
                    mat2_count == 0 &&
                    mat3_count == 0 &&
                    mat4_count == 0, "invalid recipe material count");

        uint32_t dna = random_dna(*recipe, from, code);
        add_item(from, code, dna, 1, 0);
        material_controller.remove_mats(from, mat_ids);
    }

    uint32_t random_dna(const ritem &rule, name from, int code) {
        auto rval = player_controller.begin_random(from, r4_craft, rule.grade);
        uint32_t stat1 = rval.range(101);
        uint32_t stat2 = rval.range(101);
        uint32_t stat3 = rval.range(101);
        uint32_t reveal1 = 1;
        uint32_t reveal2 = rval.range(100) < rule.stat2_reveal_rate ? 1 : 0;
        uint32_t reveal3 = rval.range(100) < rule.stat3_reveal_rate ? 1 : 0;
        player_controller.end_random(from, rval, r4_craft, rule.grade);

        uint32_t reveal = (reveal3 << 2) | (reveal2 << 1) | reveal1;
        uint32_t dna = (reveal << 24) | (stat3 << 16) | (stat2 << 8) | stat1;
        return dna;
    }

    int get_variation_value(int amount, int rate) {
        if (rate < 0) {
            rate = 0;
        }

        if (rate > 100) {
            rate = 100;
        }

        return -amount + (amount * 2) * rate / 100;
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

    uint32_t apply_bonus_stat(uint32_t stat, uint32_t bonus) {
        return stat * (bonus + 100) / 100;
    }
};
