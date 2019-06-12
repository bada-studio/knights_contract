#pragma once

/*
 * season mode material controller
 */
class smaterial_control : public material_control_base<
    smaterial_table, 
    splayer, 
    splayer_table, 
    splayer_control> {

public:
    // constructor
    //-------------------------------------------------------------------------
    smaterial_control(name _self,
                     system_control &_system_controller,
                     splayer_control &_player_controller)
            : material_control_base(_self, _system_controller, _player_controller) {
    }
};
