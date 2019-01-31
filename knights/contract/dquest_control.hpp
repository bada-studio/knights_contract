#pragma once

class dquest_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;
    player_control &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    dquest_control(account_name _self,
                   item_control &_item_controller,
                   player_control &_player_controller,
                   admin_control &_admin_controller)
        : self(_self)
        , item_controller(_item_controller)
        , player_controller(_player_controller)
        , admin_controller(_admin_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    void submitdquest(name from, uint16_t code, playerv2 &variable) {
        dquest_table table(self, self);
        if (table.cbegin() == table.cend()) {
            return;
        }

        auto last = --table.cend();
        auto current = time_util::getnow();
        if(current < last->start || last->get_end() < current) {
            return;
        }

        if (last->id != variable.dquest_no) {
            variable.dquest_no = last->id;
            variable.clear_dungeon_quest_point();
        }

        // calculation point
        auto mode = code % 100;
        auto point = variable.get_dungeon_quest_point(mode) + 5 + (code / 100) - 1;
        variable.set_dungeon_quest_point(mode, point);

        // find subquest index
        int subq_index = -1;
        for(int index = 0; index < last->subquests.size(); index++) {
            if (last->subquests[index].detail.mode == mode) {
                subq_index = index;
                break;
            }
        }

        if (subq_index == -1) {
            return;
        }

        // check it's new record
        auto &subq = last->subquests[subq_index];
        int max_count = subq.detail.max_record_count;
        int current_size = subq.records.size();
        bool new_record = false;
        if (current_size < max_count) {
            new_record = true;
        } else {
            if (subq.records[current_size-1].point < point) {
                new_record = true;
            }
        }

        if (new_record == false) {
            return;
        }

        // find place to save
        bool insert = true;
        int pos = current_size;
        for (int k = 0; k < current_size; k++) {
            auto &record = subq.records[k];
            if (record.point < point) {
                if (k < pos) {
                    pos = k;
                }
            }

            if (record.owner == from) {
                pos = k;
                insert = false;
                break;
            }
        }

        // update point
        table.modify(last, self, [&](auto &target) {
            auto &subquest = target.subquests[subq_index];
            if (insert) {
                auto record = dquestrecord();
                record.owner = from;
                record.point = point;
                record.paid = false;
                subquest.records.insert(subquest.records.cbegin() + pos, record);
                if (subquest.records.size() > max_count) {
                    subquest.records.erase(--subquest.records.cend());
                }
            } else {
                subquest.records[pos].point = point;
                std::sort(subquest.records.begin(), subquest.records.end(), 
                        [](const dquestrecord &a, const dquestrecord &b) {
                    return a.point > b.point; 
                });
            }
        });
    }

    // actions
    //-------------------------------------------------------------------------
    void adddquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        player_controller.require_coo_auth();

        dquest_table table(self, self);
        auto iter = table.find(id);

        // new one
        if (iter == table.cend()) {
            auto id = table.available_primary_key();
            if (id == 0) {
                id++;
            }

            // check last one is still opened
            if (table.cbegin() != table.cend()) {
                auto now = time_util::getnow();
                auto last = --table.cend();
                assert_true(last->get_end() < now, "there is already a dquest");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.sponsor = sponsor;
                target.start = start;
                target.duration = duration;
            });

            // add version to rule
            rversion_table rtable(self, self);
            auto ver = rtable.find(N(dquest));
            if (ver != rtable.cend()) {
                rtable.modify(ver, self, [&](auto &target) {
                    target.version = id;
                });
            } else {
                rtable.emplace(self, [&](auto &target) {
                    target.rule = to_name(N(dquest));
                    target.version = id;
                });
            }
        } else {
            // update info
            table.modify(iter, self, [&](auto& target) {
                target.sponsor = sponsor;
                target.start = start;
                target.duration = duration;
            });
        }
    }

    void removedquest(uint32_t id, bool force) {
        player_controller.require_coo_auth();

        dquest_table table(self, self);
        auto iter = table.find(id);
        assert_true(iter != table.cend(), "there is no event");

        if (force == false) {
            for (int index = 0; index < iter->subquests.size(); index++) {
                assert_true(iter->subquests[index].records.size() == 0, "there is player's record");
            }
        }

        table.erase(iter);
    }

    void updatedsubq(uint32_t id, const std::vector<dquestdetail>& details) {
        player_controller.require_coo_auth();

        dquest_table table(self, self);
        auto iter = table.find(id);
        assert_true(iter != table.cend(), "there is no event");
        
        table.modify(iter, self, [&](auto& target) {
            // first insert
            if (target.subquests.size() == 0) {
                for (int index = 0; index < details.size(); index++) { 
                    dsubquest quest;
                    quest.detail = details[index];
                    target.subquests.push_back(quest);
                }
            } else {
                // update sub quest info
                assert_true(target.subquests.size() == details.size(), "not match quest item count");
                for (int index = 0; index < target.subquests.size(); index++) {
                    target.subquests[index].detail = details[index];
                }
            }
        });
    }

    uint64_t get_code_name(eosio::symbol_type symbol) {
        switch (symbol) {
            case S(4, EOS): return N(eosio.token);
            case S(4, BADA): return N(thebadatoken);
            case S(4, TRYBE): return N(trybenetwork);
            case S(4, MEETONE): return N(eosiomeetone);
        }

        return 0;
    }

    void divdquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        player_controller.require_coo_auth();

        dquest_table table(self, self);
        auto dquest = table.find(id);
        assert_true(dquest != table.cend(), "no dquest exist");

        // finish check
        auto now = time_util::getnow();
        assert_true(dquest->get_end() < now, "dquest is still doing");

        // get a subquest
        assert_true(no >= 0 && no < dquest->subquests.size(), "incorrect no");
        auto &subquest = dquest->subquests[no];
        auto &records = subquest.records;

        long sum = 0;
        for (int index = 0; index < records.size(); index++) {
            sum += records[index].point;
        }

        int current = 0;
        asset quantity1(0, subquest.detail.reward.symbol);
        asset quantity2(0, subquest.detail.reward2.symbol);

        for (int index = from; index < records.size(); index++) {
            auto &record = records[index];
            if (record.paid) {
                continue;
            }

            // reward calculation
            asset reward = subquest.detail.reward;
            reward = reward * record.point / sum;
            quantity1 += reward;

            asset reward2 = subquest.detail.reward2;
            reward2 = reward2 * record.point / sum;
            quantity2 += reward2;

            // build message
            auto message = std::string("dquest-dividend:") + 
                        std::to_string(id) + ":" + 
                        std::to_string(sum) + ":" + 
                        std::to_string(record.point);

            // reward transfer
            if (reward.amount > 0) {
                action(permission_level{ self, N(active) },
                    get_code_name(reward.symbol), N(transfer),
                    std::make_tuple(self, record.owner, reward, message)
                ).send();
            }

            // reward2 transfer
            if (reward2.amount > 0) {
                action(permission_level{ self, N(active) },
                    get_code_name(reward2.symbol), N(transfer),
                    std::make_tuple(self, record.owner, reward2, message)
                ).send();
            }

            if (++current == count) {
                break;
            }
        }

        // write expenses log
        if (quantity1.symbol == S(4, EOS) && quantity1.amount > 0) {
            admin_controller.add_expenses(quantity1, to_name(self), "craft contest dividend to players");
        }
        if (quantity2.symbol == S(4, EOS) && quantity2.amount > 0) {
            admin_controller.add_expenses(quantity2, to_name(self), "craft contest dividend to players");
        }

        // set paid flag
        table.modify(dquest, self, [&](auto& target) {
            auto &subquest = target.subquests[no];

            int current = 0;
            for (int index = from; index < subquest.records.size(); index++) {
                auto &record = subquest.records[index];
                record.paid = true;
                if (++current == count) {
                    break;
                }
            }
        });
    }
};
