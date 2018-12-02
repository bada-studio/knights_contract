#pragma once

class dungeon_control : public control_base {
private:
    account_name self;
    material_control &material_controller;
    player_control &player_controller;
    knight_control &knight_controller;

    rule_controller<rdungeon, rdungeon_table> dungeon_rule_controller;
    rule_controller<rdgticket, rdgticket_table> dgticket_rule_controller;
    rule_controller<rmobs, rmobs_table> mobs_rule_controller;
    rule_controller<rmobskills, rmobskills_table> mobskills_rule_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    dungeon_control(account_name _self,
                    material_control &_material_controller,
                    player_control &_player_controller,
                    knight_control &_knight_controller)
        : self(_self)
        , dungeon_rule_controller(_self, N(dungeon))
        , dgticket_rule_controller(_self, N(dgticket))
        , mobs_rule_controller(_self, N(mobs))
        , mobskills_rule_controller(_self, N(mobskills))
        , material_controller(_material_controller)
        , player_controller(_player_controller)
        , knight_controller(_knight_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void dgtcraft(name from, uint16_t code, const std::vector<uint32_t> &mat_ids) {
        require_auth(from);

        // get rule
        auto &rule_table = dgticket_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "there is no dungeon rule");

        int cnt1 = rule->cnt1;
        int cnt2 = rule->cnt2;
        int cnt3 = rule->cnt3;

        // check recipe
        auto &mats = material_controller.get_materials(from);
        for (int index = 0; index < mat_ids.size(); index++) {
            auto &mat = material_controller.get_material(mats, mat_ids[index]);
            if (mat.code == rule->mat1) {
                cnt1--;
            } else if (mat.code == rule->mat2) {
                cnt2--;
            } else if (mat.code == rule->mat3) {
                cnt3--;
            } else {
                assert_true(false, "invalid ticket recipe");
            }
        }

        if (cnt1 != 0 || cnt2 != 0 || cnt3 != 0) {
            assert_true(false, "invalid ticket recipe");
        }

        // remove material
        material_controller.remove_mats(from, mat_ids);

        // add ticket
        dungeons_table table(self, self);
        auto iter = table.find(from);
        if (iter == table.cend()) {
            table.emplace(self, [&](auto& target) {
                dgticket ticket;
                ticket.code = code;
                ticket.count = 1;

                target.owner = from;
                target.tickets.push_back(ticket);
            });
        } else {
            table.modify(iter, self, [&](auto& target) {
                int tpos = target.find_ticket(code);
                if (tpos >= 0) {
                    target.tickets[tpos].count++;
                } else {
                    dgticket ticket;
                    ticket.code = code;
                    ticket.count = 1;
                    target.tickets.push_back(ticket);
                }
            });
        }
    }

    void dgenter(name from, uint16_t code) {
        require_auth(from);
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");

        // required floor check
        auto &rule_table = dungeon_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "there is no dungeon rule");
        assert_true(rule->required_floor <= player->maxfloor, "need more maxfloor");

        // get dungeon table
        dungeons_table table(self, self);
        auto iter = table.find(from);
        assert_true(table.cend() != iter, "to dungeon data");

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

    void dgleave(name from, uint16_t code) {
        require_auth(from);
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");

        // get rule
        auto &rule_table = dungeon_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "there is no dungeon rule");

        // get dungeon table
        dungeons_table table(self, self);
        auto iter = table.find(from);
        assert_true(table.cend() != iter, "to dungeon data");

        int pos = iter->find_data(code);
        assert_true(pos >= 0, "there is no dungeon");

        // remove dongeon
        table.modify(iter, self, [&](auto& target) {
            target.rows.erase(target.rows.cbegin() + pos);
        });

        // add magic water
        player_controller.increase_powder(player, rule->losemw);
    }

    void dgclear(name from, uint16_t code, const std::vector<dgorder> orders) {
        // todo: validate order
    }

    rule_controller<rdungeon, rdungeon_table>& get_dungeon_rule() {
        return dungeon_rule_controller;
    }

    rule_controller<rdgticket, rdgticket_table>& get_dgticket_rule() {
        return dgticket_rule_controller;
    }

    rule_controller<rmobs, rmobs_table>& get_mobs_rule() {
        return mobs_rule_controller;
    }

    rule_controller<rmobskills, rmobskills_table>& get_mobskills_rule() {
        return mobskills_rule_controller;
    }    
};