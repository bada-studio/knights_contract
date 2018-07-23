#pragma once

class powder_control : public control_base {
private:
    account_name self;
    rule_controller<rmpgoods, rmpgoods_table> mp_goods_rule_controller;
    player_control &player_controller;
    saleslog_control &saleslog_controller;

public:
    /// @brief
    /// Constructor
    powder_control(account_name _self,
                   player_control &_player_controller,
                   saleslog_control &_saleslog_controller)
            : self(_self)
            , mp_goods_rule_controller(_self, N(mpgoods))
            , player_controller(_player_controller)
            , saleslog_controller(_saleslog_controller) {
    }

    /// @brief
    /// Returns a controller that can CRUD the rule.
    /// @return
    /// Rule controller for magic powder goods rule
    rule_controller<rmpgoods, rmpgoods_table>& get_mpgoods_rule() {
        return mp_goods_rule_controller;
    }

    /// @brief
    /// The user can purchase magic powder through this action.
    /// Purchase history is recorded in the mpplog table.
    /// @param from
    /// account name who wants to purchase
    /// @param pid
    /// product id
    void buymp(name from, uint8_t pid, const asset &quantity) {
        require_auth(from);

        auto &rule_table = mp_goods_rule_controller.get_table();
        auto rule = rule_table.find(pid);
        assert_true(rule != rule_table.cend(), "could not find goods rule");

        auto player = player_controller.get_player(from);
        assert_true(!player_controller.is_empty_player(player), "could not find player");

        // pay the cost
        asset price = rule->price;
        assert_true(quantity.amount == price.amount, "mw price does not match");
        // player_controller.transfer(from, to_name(self), price);
        player_controller.increase_powder(player, rule->powder);

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::getnow();
        blog.type = ct_mp;
        blog.pid = rule->pid;
        blog.code = 0;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = rule->price;
        saleslog_controller.add_buylog(blog, from);
    }
};
