#pragma once

class cquest_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    cquest_control(account_name _self,
                    item_control &_item_controller,
                    admin_control &_admin_controller)
        : self(_self)
        , item_controller(_item_controller)
        , admin_controller(_admin_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void addcquest(uint16_t id, const cquestinfo& info) {
        require_auth(self);
        eosio_assert(info.reward.symbol == S(4, EOS), "only accepts EOS for deposits");
        eosio_assert(info.reward.is_valid(), "Invalid token transfer");
        eosio_assert(info.reward.amount > 0, "Quantity must be positive");

        cquest_table table(self, self);
        auto iter = table.find(id);

        // new one
        if (iter == table.cend()) {
            // check last
            if (table.cbegin() != table.cend()) {
                auto now = time_util::getnow();
                auto last = --table.cend();
                assert_true(last->info.get_end() < now, "there is already a cquest");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.count = 0;
                target.info = info;
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
            table.modify(iter, self, [&](auto& target) {
                target.info = info;
            });
        }
    }

    void submitcquest(name from, uint16_t cquest_id, uint16_t item_id) {
        require_auth(from);

        cquest_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no cquest exist");
        auto cquest = --table.cend();
        assert_true(cquest->count < cquest->info.max_count, "cquest is done!");

        auto now = time_util::getnow();
        assert_true(cquest->id == cquest_id, "incorrect cquest id");
        assert_true(cquest->info.is_cquest_period(now), "it is not cquest period");
        
        auto &items = item_controller.get_items(from);
        auto &item = item_controller.get_item(items, item_id);
        assert_true(item.id == item_id, "can not found item");
        assert_true(cquest->info.code == item.code, "incorrect item code");
        assert_true(cquest->info.level == item.level, "incorrect item level");

        int score = item_controller.calculate_item_score(item.code, item.dna);
        assert_true(cquest->info.score_from <= score, "incorrect item score");
        assert_true(cquest->info.score_to >= score, "incorrect item score");

        std::vector<uint32_t> item_ids;
        item_ids.push_back(item_id);
        item_controller.remove_items(from, item_ids);

        time current = time_util::getnow();
        int cooltime_sec = cquest->info.cooltime_min * time_util::min;

        table.modify(cquest, self, [&](auto& target) {
            bool found = false;
            for (int index = 0; index < target.rows.size(); index++) {
                auto &row = target.rows[index];
                if (row.owner == from) {
                    assert_true((current - row.at) >= cooltime_sec, "You need wait for submit cooltime.");
                    row.at = current;
                    row.count++;    
                    found = true;
                    break;
                }
            }

            if (found == false) {
                cquestrow row;
                row.owner = from;
                row.count = 1;
                row.at = current;
                row.paid = false;
                target.rows.push_back(row);
            }

            target.count++;
        });
    }

    void divcquest(uint64_t id, uint16_t from, uint16_t count) {
        require_auth(self);

        cquest_table table(self, self);
        auto cquest = table.find(id);
        assert_true(cquest != table.cend(), "no cquest exist");

        auto now = time_util::getnow();
        assert_true(cquest->info.get_end() < now, "cquest is still doing");
        int total_count = cquest->count;
        
        int current = 0;
        asset quantity(0, S(4, EOS));
        for (int index = from; index < cquest->rows.size(); index++) {
            auto &row = cquest->rows[index];
            if (row.paid) {
                continue;
            }

            // reward calculation
            asset reward = cquest->info.reward;
            reward = reward * row.count / total_count;
            quantity += reward;

            // transfer
            auto message = std::string("cquest-dividend:") + 
                        std::to_string(id) + ":" + 
                        std::to_string(total_count) + ":" + 
                        std::to_string(row.count);

            action(permission_level{ self, N(active) },
                N(eosio.token), N(transfer),
                std::make_tuple(self, row.owner, reward, message)
            ).send();

            if (++current == count) {
                break;
            }
        }

        admin_controller.add_expenses(quantity, to_name(self), "craft contest dividend");

        // set paid flag
        table.modify(cquest, self, [&](auto& target) {
            int current = 0;
            for (int index = from; index < target.rows.size(); index++) {
                auto &row = target.rows[index];
                row.paid = true;
                if (++current == count) {
                    break;
                }
            }
        });     
    }
};
