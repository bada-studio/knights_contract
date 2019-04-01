#pragma once

class itemevt_control : public control_base {
private:
    account_name self;
    player_control &player_controller;

public:
    /// @brief
    /// Constructor
    itemevt_control(account_name _self,
                    player_control &_player_controller)
            : self(_self)
            , player_controller(_player_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void getevtitem(name from) {
        // need to implement
    }

    void addevtitem(uint64_t id, uint32_t code, uint32_t from, uint32_t day) {
        player_controller.require_coo_auth();
        auto now = time_util::now();
        
        itemevt_table table(self, self);
        auto iter = table.cbegin();
        if (iter == table.cend()) {
            table.emplace(self, [&](auto &target) {
                target.id = id;
                target.code = code;
                target.from = from;
                target.duration = day * time_util::day;
            });
        } else {
            assert_true(iter->end() < now, "there is remain item event");

            table.modify(iter, self, [&](auto &target) {
                target.id = id;
                target.code = code;
                target.from = from;
                target.duration = day * time_util::day;
            });
        }
    }
};
