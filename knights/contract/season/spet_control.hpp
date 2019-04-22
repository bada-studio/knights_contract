#pragma once

/*
 * normal mode pet controller
 */
class spet_control : public pet_control_base<
    spet_table, 
    splayer, 
    splayer_table, 
    splayer_table::const_iterator, 
    splayer_control, 
    smaterial_control> {

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    spet_control(account_name _self,
                 system_control &_system_controller,
                 splayer_control &_player_controller,
                 smaterial_control &_material_controller)
            : pet_control_base(_self, 
                               _system_controller,
                               _player_controller,
                               _material_controller) {
    }
};