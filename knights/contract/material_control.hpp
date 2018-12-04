#pragma once

class material_control : public control_base {
private:
    account_name self;
    material_table materials;
    rule_controller<rmaterial, rmaterial_table> material_rule_controller;
    player_control &player_controller;

    std::vector<matrow> empty_matrows;
    matrow empty_matrow;

public:
    // constructor
    //-------------------------------------------------------------------------
    material_control(account_name _self,
                     player_control &_player_controller)
            : self(_self)
            , material_rule_controller(_self, N(material))
            , materials(_self, _self)
            , player_controller(_player_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    int get_max_inventory_size(const player& player) {
        int size = kv_material_inventory_size;
        int upgrade = player.mat_ivn_up;
        if (upgrade > kv_max_material_inventory_up) {
            upgrade = kv_max_material_inventory_up;
        }

        size += upgrade * kv_bonus_size_for_inventory_up;
        return size;
    }

    void add_material(name from, uint16_t code) {
        matrow row;
        row.code = code;
        row.saleid = 0;

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

    const std::vector<matrow>& get_materials(name from) {
        auto iter = materials.find(from);
        if (iter != materials.cend()) {
            return iter->rows;
        }

        return empty_matrows;
    }

    const matrow& get_material(const std::vector<matrow> &rows, int id) {
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
        
        assert_true(false, "can not found material");
        return empty_matrow;
    }

    void make_material_forsale(name from, uint64_t materialid, uint64_t saleid) {
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

    void new_material_from_market(name from, uint16_t code) {
        add_material(from, code);
    }

    uint32_t remove_mats(name from, const std::vector<uint32_t> &mat_ids) {
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");
        auto &mat_rule = material_rule_controller.get_table();

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
                        mat.rows.erase(mat.rows.begin() + mid);
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

    // actions
    //-------------------------------------------------------------------------
    rule_controller<rmaterial, rmaterial_table>& get_rmaterial_rule() {
        return material_rule_controller;
    }

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
