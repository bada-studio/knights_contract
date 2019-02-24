#pragma once

class variable_control : public control_base {
private:
    account_name self;

public:
    rule_controller<rvariable, rvariable_table> rvariable_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    variable_control(account_name _self)
            : self(_self)
            , rvariable_controller(_self, N(variable)) {
    }
};
