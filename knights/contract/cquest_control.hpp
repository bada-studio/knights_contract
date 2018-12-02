#pragma once

class cquest_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;
    player_control &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    cquest_control(account_name _self,
                    item_control &_item_controller,
                    player_control &_player_controller,
                    admin_control &_admin_controller)
        : self(_self)
        , item_controller(_item_controller)
        , player_controller(_player_controller)
        , admin_controller(_admin_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void addcquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        player_controller.require_coo_auth();

        cquest_table table(self, self);
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
                assert_true(last->get_end() < now, "there is already a cquest");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.sponsor = sponsor;
                target.start = start;
                target.duration = duration;
            });

            // add version to rule
            rversion_table rtable(self, self);
            auto ver = rtable.find(N(cquest));
            if (ver != rtable.cend()) {
                rtable.modify(ver, self, [&](auto &target) {
                    target.version = id;
                });
            } else {
                rtable.emplace(self, [&](auto &target) {
                    target.rule = to_name(N(cquest));
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

    void removecquest(uint32_t id, bool force) {
        player_controller.require_coo_auth();

        cquest_table table(self, self);
        auto iter = table.find(id);
        assert_true(iter != table.cend(), "there is no event");

        if (force == false) {
            for (int index = 0; index < iter->subquests.size(); index++) {
                assert_true(iter->subquests[index].records.size() == 0, "there is player's record");
            }
        }

        table.erase(iter);
    }

    void updatesubq(uint32_t id, const std::vector<cquestdetail>& details) {
        player_controller.require_coo_auth();

        cquest_table table(self, self);
        auto iter = table.find(id);
        assert_true(iter != table.cend(), "there is no event");
        
        table.modify(iter, self, [&](auto& target) {
            // first insert
            if (target.subquests.size() == 0) {
                for (int index = 0; index < details.size(); index++) { 
                    csubquest quest;
                    quest.detail = details[index];
                    quest.total_submit_count = 0;
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

    void submitcquest(name from, uint32_t cquest_id, uint8_t no, uint32_t item_id) {
        require_auth(from);

        cquest_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no cquest exist");
        auto cquest = --table.cend();

        auto now = time_util::getnow();
        assert_true(cquest->id == cquest_id, "incorrect cquest id");
        assert_true(cquest->is_cquest_period(now), "it is not in the quest period");
        assert_true(no >= 0 && no < cquest->subquests.size(), "incorrect no");

        // submit limit check
        auto &subq = cquest->subquests[no];
        assert_true(subq.total_submit_count < subq.detail.submit_limit, "cquest is done!");

        // get a target item
        auto &items = item_controller.get_items(from);
        auto &item = item_controller.get_item(items, item_id);

        // code and level check
        assert_true(item.id == item_id, "can not found item");
        assert_true(subq.detail.code == item.code, "incorrect item code");
        assert_true(subq.detail.level == item.level, "incorrect item level");

        // score check
        int score = item_controller.calculate_item_score(item.code, item.dna);
        assert_true(subq.detail.score_from <= score, "incorrect item score");
        assert_true(subq.detail.score_to >= score, "incorrect item score");

        // remove the target item
        std::vector<uint32_t> item_ids;
        item_ids.push_back(item_id);
        item_controller.remove_items(from, item_ids);

        table.modify(cquest, self, [&](auto& target) {
            bool found = false;
            auto &subquest = target.subquests[no];

            // update submit count if there is already submit record
            for (int index = 0; index < subquest.records.size(); index++) {
                auto &record = subquest.records[index];
                if (record.owner == from) {
                    // check user's limit
                    assert_true(record.submit_count < subquest.detail.submit_limit_pu, "can not exceed submit limit");
                    record.submit_count++;
                    found = true;
                    break;
                }
            }

            // add a new submit record
            if (found == false) {
                cquestrecord record;
                record.owner = from;
                record.submit_count = 1;
                record.paid = false;
                subquest.records.push_back(record);
            }

            subquest.total_submit_count++;
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

    void divcquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        player_controller.require_coo_auth();

        cquest_table table(self, self);
        auto cquest = table.find(id);
        assert_true(cquest != table.cend(), "no cquest exist");

        // finish check
        auto now = time_util::getnow();
        assert_true(cquest->get_end() < now, "cquest is still doing");

        // get a subquest
        assert_true(no >= 0 && no < cquest->subquests.size(), "incorrect no");
        auto &subquest = cquest->subquests[no];

        int total_count = subquest.total_submit_count;
        int current = 0;
        asset quantity1(0, subquest.detail.reward.symbol);
        asset quantity2(0, subquest.detail.reward2.symbol);

        for (int index = from; index < subquest.records.size(); index++) {
            auto &record = subquest.records[index];
            if (record.paid) {
                continue;
            }

            // reward calculation
            asset reward = subquest.detail.reward;
            reward = reward * record.submit_count / total_count;
            quantity1 += reward;

            asset reward2 = subquest.detail.reward2;
            reward2 = reward2 * record.submit_count / total_count;
            quantity2 += reward2;

            // build message
            auto message = std::string("cquest-dividend:") + 
                        std::to_string(id) + ":" + 
                        std::to_string(total_count) + ":" + 
                        std::to_string(record.submit_count);

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
        table.modify(cquest, self, [&](auto& target) {
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
