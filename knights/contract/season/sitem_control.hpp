#pragma once

/*
 * season mode item controller
 */
class sitem_control : public item_control_base<
    sitem_table,
    sitem_table::const_iterator, 
    smaterial_table, 
    splayer,
    splayer_table::const_iterator,
    splayer_control,
    smaterial_control> {

public:
    // constructor
    //-------------------------------------------------------------------------
    sitem_control(name _self,
                 system_control &_system_controller,
                 splayer_control &_player_controller,
                 smaterial_control &_material_controller)
            : item_control_base(
                _self,
                _system_controller,
                _player_controller,
                _material_controller) {
    }

protected:
    virtual void on_insufficient_mat_for_craft(
                splayer_table::const_iterator player, 
                int total_mat_count, 
                int code, 
                bool only_check) {
        int help_from = (kv_season_mat_factor >> 8) & 0xFF;
        int mat_price_scaler = kv_season_mat_factor & 0xFF;
        assert_true(total_mat_count >= help_from, "can not craft");
        
        rmaterial_table mat_rule(self, self.value);
        auto rule = mat_rule.find(code);
        assert_true(rule != mat_rule.cend(), "can not found material rule");
        
        int mw = rule->powder * mat_price_scaler;
        player_controller.decrease_powder(player, mw, only_check);
    }
};
