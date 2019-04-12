#pragma once

/*
 * season mode player controller
 */
class splayer_control : public player_control_base<splayer_table, splayer_table::const_iterator> {
public:
    splayer_control(account_name _self, saleslog_control &_saleslog_controller)
    : player_control_base(_self, _saleslog_controller, ct_dmw) {
    }

protected:
    virtual void on_bymp(name from, const asset &quantity) {
        // check in season
        season_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto iter = --table.cend();
        assert_true(iter->info.is_in(time_util::now()), "not in season period");

        // check overflow
        auto limit = iter->info.spending_limit.amount;
        auto player = get_player(from);
        assert_true(player != players.cend(), "can not found player");

        auto spending = player->spending + quantity;
        assert_true(spending.amount <= limit, "overflow spending limit");
        players.modify(player, self, [&](auto &target) {
            target.spending = spending;
        });
    }
};
