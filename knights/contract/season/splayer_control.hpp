#pragma once

/*
 * season mode player controller
 */
class splayer_control : public player_control_base<splayer_table, splayer_table::const_iterator> {
public:
    splayer_control(name _self, saleslog_control &_saleslog_controller)
    : player_control_base(_self, _saleslog_controller, ct_dmw) {
    }
};
