#pragma once

class skin_control : public control_base {
private:
    account_name self;

    player_control &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    skin_control(account_name _self,
                player_control &_player_controller)
            : self(_self)
            , player_controller(_player_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------

    // actions
    //-------------------------------------------------------------------------
};
