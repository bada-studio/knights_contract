#pragma once

class powder_control : public control_base {
private:
    account_name self;
    system_control &system_controller;
    saleslog_control &saleslog_controller;

public:
    rule_controller<rmpgoods, rmpgoods_table> mp_goods_rule_controller;

public:
    /// @brief
    /// Constructor
    powder_control(account_name _self,
                   system_control &_system_controller,
                   saleslog_control &_saleslog_controller)
            : self(_self)
            , mp_goods_rule_controller(_self, N(mpgoods))
            , system_controller(_system_controller)
            , saleslog_controller(_saleslog_controller) {
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

        auto player = system_controller.get_player(from);
        assert_true(!system_controller.is_empty_player(player), "could not find player");

        // pay the cost
        asset price = rule->price;
        assert_true(quantity.amount == price.amount, "mw price does not match");
        // system_controller.transfer(from, to_name(self), price);
        system_controller.increase_powder(player, rule->powder);

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::now_shifted();
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
