#pragma once

class season_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;
    player_control &player_controller;
    knight_control &knight_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    season_control(account_name _self,
                   item_control &_item_controller,
                   player_control &_player_controller,
                   knight_control &_knight_controller, 
                   admin_control &_admin_controller)
        : self(_self)
        , item_controller(_item_controller)
        , player_controller(_player_controller)
        , knight_controller(_knight_controller)
        , admin_controller(_admin_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------

    // actions
    //-------------------------------------------------------------------------
    void devreset() {
        require_auth(self);
        {
            season_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

        {
            splayer_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

        {
            sknight_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

        {
            smaterial_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

        {
            sitem_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

        {
            spet_table table(self, self);
            auto iter = table.begin();
            while (iter != table.cend()) {
                iter = table.erase(iter);
            }
        }

    }

    void addseason(uint32_t id, uint64_t start, uint32_t day, uint32_t speed, 
                   uint32_t powder, uint32_t stage, asset spending_limit, 
                   const std::vector<asset> &rewards, const std::vector<std::string> &sponsors) {
        player_controller.require_coo_auth();

        season_table table(self, self);
        auto iter = table.find(id);

        // new one
        if (iter == table.cend()) {
            auto id = table.available_primary_key();
            if (id == 0) {
                id++;
            }

            // check last one is still opened
            if (table.cbegin() != table.cend()) {
                auto now = time_util::now_shifted();
                auto last = --table.cend();
                assert_true(last->get_end() < now, "there is already a season");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.start = start;
                target.duration = day * time_util::day;
                target.speed = speed;
                target.init_powder = powder;
                target.stage = stage;
                target.spending_limit = spending_limit;
                target.rewards = rewards;
                target.sponsors = sponsors;
            });

            // add version to rule
            rversion_table rtable(self, self);
            auto ver = rtable.find(N(season));
            if (ver != rtable.cend()) {
                rtable.modify(ver, self, [&](auto &target) {
                    target.version = id;
                });
            } else {
                rtable.emplace(self, [&](auto &target) {
                    target.rule = to_name(N(season));
                    target.version = id;
                });
            }
        } else {
            // update info
            table.modify(iter, self, [&](auto& target) {
                target.start = start;
                target.duration = day * time_util::day;
                target.speed = speed;
                target.init_powder = powder;
                target.stage = stage;
                target.spending_limit = spending_limit;
                target.rewards = rewards;
                target.sponsors = sponsors;
            });
        }
    }

    void joinseason(name from) {
        season_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "no season yet");

        // check season
        auto now = time_util::now();
        auto season = --table.cend();
        assert_true(season->is_in(now), "no season period");

        // check full party
        require_auth(from);
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() == kt_count - 1, "you need all of knights");

        // initialize
        ready_player(from, season->id, season->init_powder);
        ready_knight(from, season->id);
        ready_item(from, season->id);
        ready_material(from, season->id);
        ready_pet(from, season->id);
    }

private:
    void ready_player(name from, uint32_t sid, uint32_t powder) {
        splayer_table table(self, self);
        auto iter = table.find(from);
        if (iter != table.cend()) {
            assert_true(iter->season < sid, "already in season");
            table.erase(iter);
        }

        table.emplace(self, [&](auto &target) {
            target.owner = from;
            target.season = sid;
            target.powder = powder;
        });
    }

    void ready_knight(name from, uint32_t sid) {
        sknight_table table(self, self);
        auto iter = table.find(from);
        if (iter != table.cend()) {
            table.erase(iter);
        }

        table.emplace(self, [&](auto &target) {
            target.owner = from;

            target.rows.push_back(knight_controller.new_knight(kt_knight));
            target.rows.push_back(knight_controller.new_knight(kt_archer));
            target.rows.push_back(knight_controller.new_knight(kt_mage));
        });
    }

    void ready_item(name from, uint32_t sid) {
        sitem_table table(self, self);
        auto iter = table.find(from);
        if (iter != table.cend()) {
            table.erase(iter);
        }

        table.emplace(self, [&](auto &target) {
            target.owner = from;
        });
    }

    void ready_material(name from, uint32_t sid) {
        smaterial_table table(self, self);
        auto iter = table.find(from);
        if (iter != table.cend()) {
            table.erase(iter);
        }

        table.emplace(self, [&](auto &target) {
            target.owner = from;
        });
    }

    void ready_pet(name from, uint32_t sid) {
        spet_table table(self, self);
        auto iter = table.find(from);
        if (iter != table.cend()) {
            table.erase(iter);
        }

        table.emplace(self, [&](auto &target) {
            target.owner = from;
        });
    }
};
