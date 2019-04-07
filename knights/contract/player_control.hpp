#pragma once

/*
 * base player controller
 */
template<typename tplayer_table, typename tplayer_cit>
class player_control_base : public control_base {
protected:
    account_name self;
    tplayer_table players;

public:
    // constructor
    //-------------------------------------------------------------------------
    player_control_base(account_name _self)
        : self(_self)
        , players(self, self) {
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
        // modify powder
        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }

    void decrease_powder(tplayer_cit player, uint32_t powder, bool only_check = false) {
        assert_true(player->powder >= powder, "not enough powder");
        if (only_check) {
            return;
        }

        // modify powder
        players.modify(player, self, [&](auto& target) {
            target.powder -= powder;
        });
    }
};


/*
 * normal mode player controller
 */
class player_control : public player_control_base<player_table, player_table::const_iterator> {
public:
    player_control(account_name _self)
    : player_control_base(_self) {
    }
};
