#pragma once

class smaterial_control : public material_control_base<N(smaterial)> {
public:
    // constructor
    //-------------------------------------------------------------------------
    smaterial_control(account_name _self)
            : material_control_base(_self) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    int get_max_inventory_size() {
        return kv_material_inventory_size;
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
