#pragma once

class village_control : public control_base {
private:
    account_name self;
    player_control &player_controller;
    material_control &material_controller;
    item_control &item_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    village_control(account_name _self,
                player_control &_player_controller,
                material_control &_material_controller,
                item_control &_item_controller)
            : self(_self)
            , player_controller(_player_controller)
            , material_controller(_material_controller)
            , item_controller(_item_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------

    // actions
    //-------------------------------------------------------------------------

};