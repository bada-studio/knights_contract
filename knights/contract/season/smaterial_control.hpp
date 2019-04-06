#pragma once

class smaterial_control : public drop_control_base {
private:
    account_name self;
    smaterial_table materials;

    std::vector<matrow> empty_matrows;
    matrow empty_matrow;

public:
    // constructor
    //-------------------------------------------------------------------------
    smaterial_control(account_name _self)
            : self(_self)
            , materials(_self, _self) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    int get_max_inventory_size() {
        return kv_material_inventory_size;
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

    uint32_t remove_mats(name from, const std::vector<uint32_t> &mat_ids, bool only_check = false) {
        auto iter = materials.find(from);
        assert_true(iter != materials.cend(), "could not found material");
        auto mat_rule = rmaterial_table(self, self);

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

    // actions
    //-------------------------------------------------------------------------
    void remove(name from, const std::vector<uint32_t> &mat_ids) {
        require_auth(from);

        splayer_table players(self, self);
        auto player = players.find(from);
        assert_true(player != players.cend(), "can not found player");

        uint32_t powder = remove_mats(from, mat_ids);
        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }
};
