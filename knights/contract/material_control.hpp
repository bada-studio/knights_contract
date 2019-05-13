#pragma once

class material_control_actions {
public:
    virtual void remove(name from, const std::vector<uint32_t> &mat_ids) = 0;
    virtual int get_max_inventory_size(int upgrade) = 0;
    virtual void new_material_from_market(name from, uint16_t code) = 0;
};

/*
 * base material controller
 */
template<typename material_table_name, 
         typename player_name,
         typename player_table_name,
         typename player_control_name>
class material_control_base : public drop_control_base
                            , public material_control_actions {
protected:
    account_name self;
    system_control &system_controller;
    player_control_name &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    material_control_base(account_name _self,
                          system_control &_system_controller, 
                          player_control_name &_player_controller)
            : self(_self)
            , system_controller(_system_controller)
            , player_controller(_player_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    int get_max_inventory_size(int upgrade) {
        int size = kv_material_inventory_size;
        if (upgrade > kv_max_material_inventory_up) {
            upgrade = kv_max_material_inventory_up;
        }

        size += upgrade * kv_bonus_size_for_inventory_up;
        return size;
    }

    int get_current_inventory_size(name from) {
        material_table_name materials(self, self);
        auto imat = materials.find(from);
        auto current_inventory_size = 0;
        if (imat != materials.cend()) {
            current_inventory_size = imat->rows.size();
        }
        return current_inventory_size;
    }

    void add_material(name from, uint16_t code) {
        matrow row;
        row.code = code;
        row.saleid = 0;

        material_table_name materials(self, self);
        auto iter = materials.find(from);
        if (iter == materials.cend()) {
            materials.emplace(self, [&](auto& mat){
                row.id = 1;
                mat.last_id = row.id;
                mat.owner = from;
                mat.rows.push_back(row);
            });
        } else {
            materials.modify(iter, self, [&](auto& mat){
                row.id = mat.last_id + 1;
                mat.owner = from;
                mat.last_id = row.id;
                mat.rows.push_back(row);
            });
        }
    }

    void add_materials(name from, const uint16_t mats[]) {
        material_table_name materials(self, self);
        auto iter = materials.find(from);
        if (iter == materials.cend()) {
            materials.emplace(self, [&](auto& mat){
                int id = 0;
                for (int index = 1; index < kt_count; index++) {
                    int code = mats[index];
                    if (code == 0) {
                        continue;
                    }

                    matrow row;
                    row.code = code;
                    row.saleid = 0;
                    row.id = ++id;
                    mat.rows.push_back(row);
                }

                mat.last_id = id;
                mat.owner = from;
            });
        } else {
            materials.modify(iter, self, [&](auto& mat){
                int id = mat.last_id;
                for (int index = 1; index < kt_count; index++) {
                    int code = mats[index];
                    if (code == 0) {
                        continue;
                    }

                    matrow row;
                    row.code = code;
                    row.saleid = 0;
                    row.id = ++id;
                    mat.rows.push_back(row);
                }

                mat.last_id = id;
                mat.owner = from;
            });
        }
    }

    uint32_t remove_mats(name from, const std::vector<uint32_t> &mat_ids, bool only_check = false) {
        material_table_name materials(self, self);
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");
        rmaterial_table mat_rule(self, self);

        uint32_t powder = 0;
        int found = false;
        materials.modify(iter, self, [&](auto& mat){
            for (int index = 0; index < mat_ids.size(); ++index) {
                found = false;
                int id = mat_ids[index];

                // binary search
                auto &rows = mat.rows;
                int left = 0;
                int right = rows.size() - 1;
                while (left <= right) {
                    int mid = left + (right - left) / 2;
                    if (rows[mid].id < id) {
                        left = mid + 1;
                    } else if (id < rows[mid].id) {
                        right = mid - 1;
                    } else {
                        // it is on sale
                        if (mat.rows[mid].saleid != 0) {
                            break;
                        }

                        // find powder rule
                        auto rule = mat_rule.find(mat.rows[mid].code);
                        if (rule == mat_rule.cend()) {
                            break;
                        }

                        powder += rule->powder;
                        if (only_check == false) {
                            mat.rows.erase(mat.rows.begin() + mid);
                        }
                        found = true;
                        break;
                    }
                }

                if (found == false) {
                    break;
                }
            }
        });

        assert_true(found, "could not found material");
        return powder;
    }

    void new_material_from_market(name from, uint16_t code) {
        add_material(from, code);
    }

    // actions
    //-------------------------------------------------------------------------
    void remove(name from, const std::vector<uint32_t> &mat_ids) {
        require_auth(from);

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "can not found player");

        uint32_t powder = remove_mats(from, mat_ids);
        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }
};


/*
 * normal mode material controller
 */
class material_control : public material_control_base<
    material_table, 
    player, 
    player_table, 
    player_control> {

public:
    // constructor
    //-------------------------------------------------------------------------
    material_control(account_name _self,
                     system_control &_system_controller,
                     player_control &_player_controller)
            : material_control_base(_self, _system_controller, _player_controller) {
    }
    
    void alchemist(name from, 
                   uint32_t grade, 
                   const std::vector<uint32_t>& mat_ids, 
                   uint32_t checksum, 
                   bool delay) {
        auto pvsi = system_controller.get_playervs(from);

        if (delay && USE_DEFERRED == 1) {
            require_auth(from);
            delay = system_controller.set_deferred(pvsi);

            if (do_alchemist(from, grade, mat_ids, delay, pvsi)) {
                eosio::transaction out{};
                out.actions.emplace_back(
                    permission_level{ self, N(active) }, 
                    self, N(alchemisti), 
                    std::make_tuple(from, grade, mat_ids, checksum)
                );
                out.delay_sec = 1;
                out.send(system_controller.get_last_trx_hash(), self);
                return;
            }
        } else {
            if (USE_DEFERRED == 1) {
                require_auth(self);
            } else {
                require_auth(from);
            }

            do_alchemist(from, grade, mat_ids, false, pvsi);
        }
    }

    bool do_alchemist(name from, uint32_t grade, const std::vector<uint32_t>& mat_ids, bool only_check, playerv2_table::const_iterator pvsi) {
        system_controller.require_action_count(1);

        auto variable = *pvsi;
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(player != players.cend(), "can not found player");

        int min_count = kv_alchemist_count & 0xFF;
        int max_count = (kv_alchemist_count >> 8) & 0xFF;
        assert_true(mat_ids.size() >= min_count, "invalid mat count");
        assert_true(mat_ids.size() <= max_count, "invalid mat count");
        assert_true(grade >= ig_rare, "invalid grade");
        assert_true(grade <= ig_legendary, "invalid grade");

        material_table materials(self, self);
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "can not found material");
        double rate = calculate_alchemist_rate(grade, *iter, mat_ids);
        if (only_check) {
            return only_check;
        }

        auto rval = system_controller.begin_random(variable);
        auto success = (rval.range(10000) < rate * 10000);
        remove_mats(from, mat_ids);

        int code = 1;
        if (success) {
            code = get_bottie(grade, rval);
        } else {
            code = get_bottie(grade - 1, rval);
        }

        add_material(from, code);
        system_controller.end_random(variable, rval);

        variable.clear_deferred_time();
        system_controller.update_playerv(pvsi, variable);
        return only_check;
    }

    void make_material_forsale(name from, uint64_t materialid, uint64_t saleid) {
        material_table materials(self, self);
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");

        bool found = false;
        materials.modify(iter, self, [&](auto& mat){
            for (int index = 0; index < mat.rows.size(); index++) {
                if (mat.rows[index].id == materialid) {
                    mat.rows[index].saleid = saleid;
                    found = true;
                    break;
                }
            }
        });

        assert_true(found, "could not found material");
    }

    void cancel_sale(name from, uint64_t saleid) {
        material_table materials(self, self);
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");

        bool found = false;
        materials.modify(iter, self, [&](auto& mat){
            for (int index = 0; index < mat.rows.size(); index++) {
                if (mat.rows[index].saleid == saleid) {
                    mat.rows[index].saleid = 0;
                    found = true;
                    break;
                }
            }
        });

        assert_true(found, "could not found material");
    }

    void remove_salematerial(name from, uint64_t saleid) {
        material_table materials(self, self);
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");

        int found = -1;
        materials.modify(iter, self, [&](auto& mat){
            for (int index = 0; index < mat.rows.size(); index++) {
                if (mat.rows[index].saleid == saleid) {
                    found = index;
                    break;
                }
            }

            if (found >= 0) {
                mat.rows.erase(mat.rows.begin() + found);
            }
        });

        assert_true(found >= 0, "could not found material");
    }

private:
    int get_grade_base_rate(int grade) {
        switch (grade) {
            case ig_normal: return drop_rates_raw[0];
            case ig_rare: return drop_rates_raw[4];
            case ig_unique: return drop_rates_raw[7];
            case ig_legendary: return drop_rates_raw[9];
            case ig_ancient: return drop_rates_raw[10];
        }

        return 0;
    }

    int get_alchemist_rate(int grade) {
        int data = kv_alchemist_rate;
        int shift = 0;

        if (grade == ig_unique) {
            shift = 8;
        } else if (grade == ig_legendary) {
            shift = 16;
        }
        
        return (data >> shift) & 0xFF;
    }

    double calculate_alchemist_rate(int grade, const material &matset, const std::vector<uint32_t>& mat_ids) {
        int normal_rate = get_grade_base_rate(ig_normal);
        int next_rate = normal_rate / get_grade_base_rate(grade);
        double failure = 1.0;

        for (int index = 0; index < mat_ids.size(); index++) {
            auto &mat = matset.get_material(mat_ids[index]);
            auto current_grade = get_field_material_grade(mat.code);
            assert_true((int)current_grade == (int)grade-1, "it has a invalid grade material");
            assert_true(mat.saleid == 0, "can not use on sale material");
            assert_true(mat.code > 0, "invalid material");

            int rate = normal_rate / drop_rates_raw[(mat.code % 20) - 1];
            failure *= 1.0 - (rate / (double) next_rate);
        }

        double rate = (1 - failure) * (get_alchemist_rate(grade) / 100.0);
        return rate;
    }    
};
