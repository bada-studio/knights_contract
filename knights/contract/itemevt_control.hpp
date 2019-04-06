#pragma once

class itemevt_control : public control_base {
private:
    account_name self;
    player_control &player_controller;
    item_control &item_controller;

public:
    /// @brief
    /// Constructor
    itemevt_control(account_name _self,
                    player_control &_player_controller,
                    item_control &_item_controller)
            : self(_self)
            , player_controller(_player_controller)
            , item_controller(_item_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void getevtitem(name from) {
        require_auth(N(bastetbastet));

        // get player info
        auto player = player_controller.get_player(from);
        assert_true(!player_controller.is_empty_player(player), "no player");

        auto pvsi = player_controller.get_playervs(from, true);
        auto variable = *pvsi;

        // get event
        itemevt_table table(self, self);
        auto iter = table.cbegin();
        assert_true(iter != table.cend(), "no item event");

        // validation
        auto now = time_util::now();
        assert_true(iter->is_in(now), "no event now");
        assert_true(iter->id != variable.itemevt, "you've already got event item");

        // get rule
        ritem_table rule_table(self, self);
        auto recipe = rule_table.find(iter->code);
        assert_true(recipe != rule_table.cend(), "can not found item");

        // add item
        uint32_t dna = item_controller.random_dna(*recipe, from, iter->code, variable);
        item_controller.add_item(from, iter->code, dna, 1, 0);

        // update event
        variable.itemevt = iter->id;
        player_controller.update_playerv(pvsi, variable);
    }

    void addevtitem(uint64_t id, uint32_t code, uint32_t from, uint32_t day) {
        player_controller.require_coo_auth();
        auto now = time_util::now();
        
        ritem_table rule_table(self, self);
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "can not found item");

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
            table.modify(iter, self, [&](auto &target) {
                target.id = id;
                target.code = code;
                target.from = from;
                target.duration = day * time_util::day;
            });
        }
    }
};
