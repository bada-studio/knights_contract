#pragma once

class season_control : public control_base {
private:
    account_name self;
    item_control &item_controller;
    admin_control &admin_controller;
    player_control &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    season_control(account_name _self,
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

    // actions
    //-------------------------------------------------------------------------
    void addseason(uint32_t id, uint32_t start, uint32_t duration,
                   uint32_t speed, uint32_t share, uint32_t rankcnt) {

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
                target.duration = duration;
                target.speed = speed;
                target.share = share;
                target.rankcnt = rankcnt;
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
                target.duration = duration;
                target.speed = speed;
                target.share = share;
                target.rankcnt = rankcnt;
            });
        }
    }

    void joinseason(name owner) {
    }

    void getsreward(name owner, const std::vector<uint32_t>& item_ids) {
    }
};
