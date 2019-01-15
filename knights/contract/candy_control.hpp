#pragma once

class candy_control : public control_base {
private:
    account_name self;
    player_control &player_controller;

public:
    /// @brief
    /// Constructor
    candy_control(account_name _self,
                  player_control &_player_controller)
            : self(_self)
            , player_controller(_player_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void getcandy(name from, const std::string &memo) {
        require_auth(N(prochaintech));
        
        auto player = player_controller.get_player(from);
        assert_true(!player_controller.is_empty_player(player), "can not find player");

        candybox_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no candy data yet");
        
        auto iter = --table.cend();
        assert_true(iter->remain >= iter->amount, "we have run out of magic water");

        table.modify(iter, self, [&](auto &target) {
            target.remain -= target.amount;
        });

        player_controller.increase_powder(player, iter->amount);
    }

    void addcandy(uint32_t total, uint32_t amount) {
        require_auth(self);
        
        candybox_table table(self, self);
        
        auto no = table.available_primary_key();
        if (table.cbegin() != table.cend()) {
            auto iter = --table.cend();
            assert_true(iter->remain < iter->amount, "there is remain candy");
        }

        table.emplace(self, [&](auto &target) {
            target.id = no;
            target.total = total;
            target.remain = total;
            target.amount = amount;
        });
    }
};