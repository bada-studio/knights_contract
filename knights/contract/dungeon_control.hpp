#pragma once

class dungeon_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;
    player_control &player_controller;
    knight_control &knight_controller;

    rule_controller<rdungeon, rdungeon_table> dungeon_rule_controller;
    rule_controller<rmobs, rmobs_table> mobs_rule_controller;
    rule_controller<rmobskills, rmobskills_table> mobskills_rule_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    dungeon_control(account_name _self,
                    item_control &_item_controller,
                    player_control &_player_controller,
                    knight_control &_knight_controller,
                    admin_control &_admin_controller)
        : self(_self)
        , dungeon_rule_controller(_self, N(dungeon))
        , mobs_rule_controller(_self, N(mobs))
        , mobskills_rule_controller(_self, N(mobskills))
        , item_controller(_item_controller)
        , player_controller(_player_controller)
        , knight_controller(_knight_controller)
        , admin_controller(_admin_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void dgenter(name from, uint16_t code) {
        require_auth(from);
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");

        dungeons_table table(self, self);
        auto iter = table.find(from);
        assert_true(table.cend() != iter, "to dungeon ticket");

        // required floor check
        auto &rule_table = dungeon_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "there is no dungeon rule");
        assert_true(rule->required_floor <= player->maxfloor, "need more maxfloor");

        // check ticket
        int tpos = iter->find_ticket(rule->tkcode);
        assert_true(tpos >= 0, "not enough ticket");
        assert_true(iter->tickets[tpos].count >= rule->tkcount, "not enough ticket");

        // check no dungeon
        int pos = iter->find_data(code);
        assert_true(pos < 0, "there is already opened dungeon");

        // get knights
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() == 3, "3 knights needed");

        // new seed
        auto rval = player_controller.begin_random(from, r4_dungeon, 0);
        player_controller.random_range(rval, 100);
        player_controller.end_random(from, rval, r4_dungeon, 0);

        dgdata data;
        data.code = code;
        data.seed = rval.seed;
        data.open = time_util::getnow();
        data.v1 = 0;
        data.v2 = 0;

        // add knights
        for (int index = 0; index < knights.size(); index++) {
            auto &knight = knights[index];
            dgknight item;
            item.type = knight.type;
            item.attack = knight.attack;
            item.defense = knight.defense;
            item.hp = knight.hp;

            // add skills
            auto &skills = knight_controller.get_knight_skills(from, knight.type);
            assert_true(skills.size() >= 5, "every knights needs to have 5 or more skills");

            for (int k = 0; k < skills.size(); k++) {
                item.skills.push_back(skills[index]);
            }

            data.knts.push_back(item);
        }

        // add dungeon data
        table.modify(iter, self, [&](auto& target) {
            target.tickets[tpos].count -= rule->tkcount;
            target.rows.push_back(data);
        });
    }

    rule_controller<rdungeon, rdungeon_table>& get_dungeon_rule() {
        return dungeon_rule_controller;
    }

    rule_controller<rmobs, rmobs_table>& get_mobs_rule() {
        return mobs_rule_controller;
    }

    rule_controller<rmobskills, rmobskills_table>& get_mobskills_rule() {
        return mobskills_rule_controller;
    }    
};