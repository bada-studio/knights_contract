#pragma once

class contest_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    contest_control(account_name _self,
                    item_control &_item_controller,
                    admin_control &_admin_controller)
        : self(_self)
        , item_controller(_item_controller)
        , admin_controller(_admin_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void addcontest(uint16_t id, const conmission& mission) {
        require_auth(self);
        eosio_assert(mission.reward.symbol == S(4, EOS), "only accepts EOS for deposits");
        eosio_assert(mission.reward.is_valid(), "Invalid token transfer");
        eosio_assert(mission.reward.amount > 0, "Quantity must be positive");

        contest_table table(self, self);
        auto iter = table.find(id);

        // new one
        if (iter != table.cend()) {
            // check last
            if (table.cbegin() != table.cend()) {
                auto now = time_util::getnow();
                auto last = --table.cend();
                assert_true(last->mission.get_end() < now, "thre is already a contest");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.count = 0;
                target.mission = mission;
            });
        } else {
            table.modify(iter, self, [&](auto& target) {
                target.mission = mission;
            });
        }
    }

    void applycontest(name from, uint16_t contest_id, uint16_t item_id) {
        require_auth(from);

        contest_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no contest exist");
        auto contest = --table.cend();
        assert_true(contest->count < contest->mission.max_count, "contest is done!");

        auto now = time_util::getnow();
        assert_true(contest->id == contest_id, "incorrect contest id");
        assert_true(contest->mission.is_contest_period(now), "it is not contest period");
        
        auto &items = item_controller.get_items(from);
        auto &item = item_controller.get_item(items, item_id);
        assert_true(item.id == item_id, "can not found item");
        assert_true(contest->mission.code == item.code, "incorrect item id");
        assert_true(contest->mission.level == item.level, "incorrect item level");

        int score = item_controller.calculate_item_score(item.code, item.dna);
        assert_true(contest->mission.score_from <= score, "incorrect item score");
        assert_true(contest->mission.score_to >= score, "incorrect item score");

        std::vector<uint32_t> item_ids;
        item_ids.push_back(item_id);
        item_controller.remove_items(from, item_ids);

        table.modify(contest, self, [&](auto& target) {
            bool found = false;
            for (int index = 0; index < target.rows.size(); index++) {
                auto &row = target.rows[index];
                if (row.owner == from) {
                    row.count++;    
                    found = true;
                    break;
                }
            }

            if (found == false) {
                contestrow row;
                row.owner = from;
                row.count = 1;
                row.paid = false;
                target.rows.push_back(row);
            }

            target.count++;
        });
    }

    void divcontest(uint64_t id, uint16_t from, uint16_t count) {
        require_auth(self);

        contest_table table(self, self);
        auto contest = table.find(id);
        assert_true(contest != table.cend(), "no contest exist");

        auto now = time_util::getnow();
        assert_true(contest->mission.get_end() < now, "contest is still doing");
        int total_count = contest->count;
        
        int current = 0;
        asset quantity(0, S(4, EOS));
        for (int index = from; index < contest->rows.size(); index++) {
            auto &row = contest->rows[index];
            if (row.paid) {
                continue;
            }

            // reward calculation
            asset reward = contest->mission.reward;
            reward = reward * row.count / total_count;
            quantity += reward;

            // transfer
            auto message = std::string("contest-dividend:") + 
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

        admin_controller.add_contest_expenses(quantity);

        // set paid flag
        table.modify(contest, self, [&](auto& target) {
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
