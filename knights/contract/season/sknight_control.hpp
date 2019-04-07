#pragma once

/*
 * season mode knight controller
 */
class sknight_control : public knight_control_base<
    sknight_table, 
    sknight_table::const_iterator, 
    splayer,
    splayer_table::const_iterator,
    splayer_control,
    smaterial_control,
    sitem_control,
    spet_control,
    N(rebirth2s)> {

public:
    // constructor
    //-------------------------------------------------------------------------
    sknight_control(account_name _self,
                    system_control &_system_controller,
                    splayer_control &_player_controller,
                    smaterial_control &_material_controller,
                    sitem_control &_item_controller,
                    spet_control &_pet_controller)
            : knight_control_base(
                _self,
                _system_controller,
                _player_controller,
                _material_controller,
                _item_controller,
                _pet_controller) {
    }
};