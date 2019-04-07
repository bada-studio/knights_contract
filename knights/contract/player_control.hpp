#pragma once

/*
 * base player controller
 */
template<typename tplayer_table, typename tplayer_cit>
class player_control_base : public control_base {
protected:
    account_name self;
    tplayer_table players;
    saleslog_control &saleslog_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    player_control_base(account_name _self, 
                        saleslog_control &_saleslog_controller)
        : self(_self)
        , players(self, self)
        , saleslog_controller(_saleslog_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    tplayer_table& get_players() {
        return players;
    }

    tplayer_cit get_player(name player_name) {
        return players.find(player_name);
    }
    
    bool is_empty_player(tplayer_cit player) {
        return player == players.end();
    }

    void increase_powder(tplayer_cit player, uint32_t powder) {
        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }

    void decrease_powder(tplayer_cit player, uint32_t powder, bool only_check = false) {
        assert_true(player->powder >= powder, "not enough powder");
        if (only_check) {
            return;
        }

        players.modify(player, self, [&](auto& target) {
            target.powder -= powder;
        });
    }

    // action
    //-------------------------------------------------------------------------
    /// @brief
    /// The user can purchase magic powder through this action.
    /// Purchase history is recorded in the mpplog table.
    /// @param from
    /// account name who wants to purchase
    /// @param pid
    /// product id
    void buymp(name from, uint8_t pid, const asset &quantity) {
        require_auth(from);

        rmpgoods_table rule_table(self, self);
        auto rule = rule_table.find(pid);
        assert_true(rule != rule_table.cend(), "could not find goods rule");

        auto player = get_player(from);
        assert_true(!is_empty_player(player), "could not find player");

        // pay the cost
        asset price = rule->price;
        assert_true(quantity.amount == price.amount, "mw price does not match");
        increase_powder(player, rule->powder);

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::now_shifted();
        blog.type = ct_mp; // todo
        blog.pid = rule->pid;
        blog.code = 0;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = rule->price;
        saleslog_controller.add_buylog(blog, from);
    }    
};


/*
 * normal mode player controller
 */
class player_control : public player_control_base<player_table, player_table::const_iterator> {
public:
    player_control(account_name _self, saleslog_control &_saleslog_controller)
    : player_control_base(_self, _saleslog_controller) {
    }
};
