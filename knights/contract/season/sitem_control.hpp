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
    sitem_control(account_name _self,
                 system_control &_system_controller,
                 splayer_control &_player_controller,
                 smaterial_control &_material_controller)
            : item_control_base(
                _self,
                _system_controller,
                _player_controller,
                _material_controller) {
    }
};
