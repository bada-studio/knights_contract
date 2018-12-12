#pragma once

class dungeon_control : public drop_control_base {
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

    void dgfreetk(name from, uint16_t code) {
        require_auth(from);

        // get player
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        auto time_now = time_util::getnow();

        // get variable
        auto free_ticket_duration = kv_dungeon_free_ticket >> 8;
        auto max_free_count = kv_dungeon_free_ticket & 0xFF;
        
        // add ticket
        dungeons_table table(self, self);
        auto iter = table.find(from);
        if (iter == table.cend()) {
            table.emplace(self, [&](auto& target) {
                dgticket ticket;
                ticket.code = code;
                ticket.count = 0;
                ticket.free_at = time_now;
                ticket.free_count = 1;

                target.owner = from;
                target.tickets.push_back(ticket);
            });
        } else {
            table.modify(iter, self, [&](auto& target) {
                int tpos = target.find_ticket(code);
                if (tpos >= 0) {
                    auto &ticket = target.tickets[tpos];
                    auto diff = time_now - ticket.free_at;
                    assert_true(ticket.free_count < max_free_count, "free ticket is full");
                    assert_true(diff > time_util::hour * free_ticket_duration, "need more time to get one");
                    ticket.free_at = time_now;
                    ticket.free_count++;
                } else {
                    dgticket ticket;
                    ticket.code = code;
                    ticket.free_at = time_now;
                    ticket.free_count = 1;
                    target.tickets.push_back(ticket);
                }
            });
        }
    }

    void dgenter(name from, uint16_t code) {
        assert_true(false, "not ready yet");

        require_auth(from);
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        auto time_now = time_util::getnow();

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
        assert_true(iter->tickets[tpos].get_total_count() >= rule->tkcount, "not enough ticket");

        int rpos = iter->find_record(code);

        // check no dungeon
        int pos = iter->find_data(code);
        assert_true(pos < 0, "there is already opened dungeon");

        // get knights
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() == 3, "3 knights needed");

        dgdata data;
        data.code = code;

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
                item.skills.push_back(skills[k]);
            }

            data.knts.push_back(item);
        }

        // add dungeon data
        table.modify(iter, self, [&](auto& target) {
            // add record
            if (rpos >= 0) {
                target.records[rpos].id++;
                target.records[rpos].at = time_now;
            } else {
                dgrecords record;
                record.id = 1;
                record.at = time_now;
                target.records.push_back(record);
            }

            // remove ticket
            target.tickets[tpos].reduce_count(rule->tkcount);

            // set seed
            auto key = player_controller.get_checksum_key(from);
            data.seed = (uint32_t)((from + time_now) ^ key);

            // add dungeon
            target.rows.push_back(data);
        });
    }

    void dgleave(name from, uint16_t code) {
        assert_true(false, "not ready yet");

        require_auth(from);
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        auto time_now = time_util::getnow();

        // get dungeon table
        dungeons_table table(self, self);
        auto iter = table.find(from);
        assert_true(table.cend() != iter, "to dungeon data");

        int rpos = iter->find_record(code);

        int pos = iter->find_data(code);
        assert_true(pos >= 0, "there is no dungeon");

        // get rule
        auto &rule_table = dungeon_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "there is no dungeon rule");

        // remove dongeon
        table.modify(iter, self, [&](auto& target) {
            if (rpos >= 0) {
                target.records[rpos].at = time_now;
                target.records[rpos].lose++;
            }

            target.rows.erase(target.rows.cbegin() + pos);
        });

        // add magic water
        player_controller.increase_powder(player, rule->losemw);
    }

    void dgclear(name from, uint16_t code, const std::vector<dgorder> orders) {
        assert_true(false, "not ready yet");

        require_auth(from);

        // get player
        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "could not find player");
        auto time_now = time_util::getnow();

        // check inventory size;
        auto &mats = material_controller.get_materials(from);
        int exp_mat_count = mats.size() + 1;
        int max_mat_count = material_controller.get_max_inventory_size(*player);
        assert_true(exp_mat_count <= max_mat_count, "insufficient inventory");

        // validate user's action
        assert_true(validate(from, code, orders), "validation failure");
        
        // get rule
        auto &rule_table = dungeon_rule_controller.get_table();
        auto rule = rule_table.find(code);
        assert_true(rule_table.cend() != rule, "could not dungeon rule");
        
        // determin drop material
        auto rval = player_controller.begin_random(from, r4_dungeon, 0);
        auto value = rval.range(10000);
        uint16_t matcode = 0;

        if (value < rule->mdrop3) {
            matcode = rule->mat3;
        } else if (value < (rule->mdrop3 + rule->mdrop2)) {
            matcode = rule->mat2;
        } else if (value < (rule->mdrop3 + rule->mdrop2 + rule->mdrop1)) {
            matcode = rule->mat1;
        } else {
            auto grade = ig_rare;
            auto value2 = rval.range(10000);
            if (value < rule->legendary_drop) {
                grade = ig_legendary;
            } else if (value < (rule->legendary_drop + rule->unique_drop)) {
                grade = ig_unique;
            }

            matcode = get_bottie(*player, grade, rval);
        }

        // add materials
        material_controller.add_material(from, matcode);

        // get dungeon table
        dungeons_table table(self, self);
        auto iter = table.find(from);
        assert_true(table.cend() != iter, "to dungeon data");
        auto pos = iter->find_data(code);
        assert_true(pos >= 0, "can not find dungeon");

        // update dungeon data
        table.modify(iter, self, [&](auto& target) {
            auto rpos = target.find_record(code);
            target.records[rpos].at = time_now;
            target.rows.erase(target.rows.begin() + pos);
        });

        player_controller.end_random(from, rval, r4_dungeon, 0);
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

private:
    bool validate(name from, uint16_t code, const std::vector<dgorder> orders) {
        // todo implement
        return true;
    }
};