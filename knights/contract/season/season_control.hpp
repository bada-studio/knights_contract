#pragma once

class season_control : public control_base {
private:
    name self;
    system_control &system_controller;
    admin_control &admin_controller;
    sknight_control &knight_controller;
    sitem_control &item_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    season_control(name _self,
                   system_control &_system_controller,
                   admin_control &_admin_controller,
                   sknight_control &_knight_controller, 
                   sitem_control &_item_controller)
        : self(_self)
        , system_controller(_system_controller)
        , admin_controller(_admin_controller)
        , knight_controller(_knight_controller)
        , item_controller(_item_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------

    // actions
    //-------------------------------------------------------------------------
    /// @brief
    /// add new season
    /// @param id
    /// Season id
    /// @param info
    /// Season info
    void addseason(bool add, const seasoninfo &info) {
        system_controller.require_coo_auth();
        season_table table(self, self.value);

        // new one
        if (add) {
            uint64_t id = table.available_primary_key();
            if (id == 0) {
                id++;
            }

            // check last one is still opened
            if (table.cbegin() != table.cend()) {
                auto now = time_util::now();
                auto last = --table.cend();
                assert_true(last->info.get_end() < now, "there is already a season");
            }

            table.emplace(self, [&](auto& target) {
                target.id = id;
                target.info = info;
            });

            // add version to rule
            rversion_table rtable(self, self.value);
            auto ver = rtable.find("season"_n.value);
            if (ver != rtable.cend()) {
                rtable.modify(ver, self, [&](auto &target) {
                    target.version = id;
                });
            } else {
                rtable.emplace(self, [&](auto &target) {
                    target.rule = "season"_n;
                    target.version = id;
                });
            }
        } else {
            auto iter = --table.cend();
            auto now = time_util::now();
            if (iter->state.records.size() > 0) {
                assert_true(now < iter->info.get_end(), "it's over");
            }

            // update info
            table.modify(iter, self, [&](auto& target) {
                target.info = info;
            });
        }
    }

    /// @brief
    /// Join new season
    /// @param from
    /// Player who want to join the season
    void joinseason(name from) {
        require_auth(from);

        // check season
        season_table table(self, self.value);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto now = time_util::now();
        auto season = --table.cend();
        assert_true(season->info.is_in(now), "no season period");

        // check full party
        knight_table ktable(self, self.value);
        auto iknt = ktable.find(from.value);
        assert_true(iknt != ktable.cend(), "no knights");
        assert_true(iknt->rows.size() == kt_count - 1, "you need all of knights");

        // check last season closing
        playerv2_table pvtable(self, self.value);
        auto pvi = pvtable.find(from.value);
        if (pvi == pvtable.cend()) {
            system_controller.new_playervs(from, 0, 0);
            pvi = pvtable.find(from.value);
        }

        assert_true(pvi->last_start_season != season->id, "already in season");
        if (pvi->last_start_season > 0) {
            assert_true(pvi->last_start_season == pvi->last_end_season, "receive last season reward first!");
        }

        pvtable.modify(pvi, self, [&](auto &target) {
            target.last_start_season = season->id;
        });

        // initialize
        remove_player(from);
        remove_knight(from);
        remove_item(from);
        remove_material(from);
        remove_pet(from);
        ready_player(from, season->id, season->info);
        ready_knight(from, season->id);
    }

    /// @brief
    /// Get season reward
    /// @param from
    /// Player who want to receive reward
    /// @param id
    /// season id
    void seasonreward(name from, int32_t id) {
        require_auth(from);
        season_table stable(self, self.value);
        auto season = stable.find(id);
        assert_true(season != stable.cend(), "invalid season id");
        auto &info = season->info;
        auto &state = season->state;
        auto now = time_util::now();
        assert_true(info.get_end() < now, "season not ended");

        // modify season end
        playerv2_table pvtable(self, self.value);
        auto pvi = pvtable.find(from.value);
        assert_true(pvi != pvtable.cend(), "can not found play log");
        assert_true(pvi->last_start_season == season->id, "invalid request");
        assert_true(pvi->last_end_season != season->id, "already got reward");
        pvtable.modify(pvi, self, [&](auto &target) {
            target.last_end_season = season->id;
        });

        splayer_table sptable(self, self.value);
        auto splayer = sptable.find(from.value);
        assert_true(splayer != sptable.cend(), "can not found play log");
        auto floor = splayer->maxfloor;
        auto rank = state.get_rank(from);
        sptable.erase(splayer);

        remove_knight(from);
        remove_item(from);
        remove_material(from);
        remove_pet(from);

        // calculate magic water
        auto mw = floor;
        if (mw < info.min_reward_powder) {
            mw = info.min_reward_powder;
        }
        if (mw > info.max_reward_powder) {
            mw = info.max_reward_powder;
        }

        player_table ptable(self, self.value);
        auto player = ptable.find(from.value);
        assert_true(player != ptable.cend(), "can not found player");
        ptable.modify(player, self, [&](auto &target) {
            target.powder += mw;
        });

        // is in rank
        if (0 < rank) {
            if (rank <= info.rewardcnt) {
                // todo send eos to player
                action(permission_level{ self, "active"_n },
                    get_code_name(info.reward.symbol), "transfer"_n,
                    std::make_tuple(self, from, info.reward, std::string("EK. Cup reward"))
                ).send();

                admin_controller.add_expenses(info.reward, self, "EK. Cup reward");
    
                if (rank == 1) {
                    give_medal(from, k_medal_gold);
                } else if (rank == 2) {
                    give_medal(from, k_medal_silver);
                } else if (rank == 3) {
                    give_medal(from, k_medal_bronze);
                } else {
                    give_medal(from, k_medal_master);
                }
            } else {
                give_medal(from, k_medal_knight);
            }
        }

        if (state.cqwinner == from) {
            give_medal(from, k_medal_death);

            // todo send eos to player
            action(permission_level{ self, "active"_n },
                get_code_name(info.quest.reward.symbol), "transfer"_n,
                std::make_tuple(self, from, info.quest.reward, std::string("EK. Cup reward"))
            ).send();
            admin_controller.add_expenses(info.quest.reward, self, "EK. Cup reward");
        }
    }

    void give_medal(name from, int8_t id) {
        medal_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter == table.cend()) {
            table.emplace(self, [&](auto &target) {
                medalrow medal;
                medal.id = id;
                medal.count = 1;

                target.owner = from;
                target.medals.push_back(medal);
            });
        } else {
            table.modify(iter, self, [&](auto &target) {
                bool found = false;
                for (int index = 0; index < target.medals.size(); index++) {
                    auto &medal = target.medals[index];
                    if (medal.id == id) {
                        medal.count++;
                        found = true;
                    }
                }

                if (found == false) {
                    medalrow medal;
                    medal.id = id;
                    medal.count = 1;

                    target.owner = from;
                    target.medals.push_back(medal);
                }
            });
        }
    }

    /// @brief
    /// Submit season quest
    /// @param from
    /// Player who want to submit the quest
    /// @param id
    /// item id
    uint8_t submitsq(name from, int32_t id) {
        require_auth(from);

        season_table table(self, self.value);
        auto iter = --table.cend();
        auto &info = iter->info;
        auto &state = iter->state;

        // validate item
        assert_true(state.cqwinner.value == 0, "already has winner");
        auto &items = item_controller.get_items(from);
        auto item = item_controller.get_item(items, id);
        auto knt = item.knight;
        assert_true(item.code == info.quest.code, "invalid quest item");
        assert_true(item.level == info.quest.level, "invalid quest item");

        // remove item
        std::vector<uint32_t> item_ids;
        item_ids.push_back(id);
        item_controller.remove_items(from, item_ids);

        // write winner
        table.modify(iter, self, [&](auto& target) {
            target.state.cqwinner = from;
        });

        return knt;
    }

    void add_revenue(asset quantity) {
        season_table table(self, self.value);
        auto iter = --table.cend();

        table.modify(iter, self, [&](auto& target) {
            target.state.dmw += quantity;
        });
    }

private:
    void ready_player(name from, uint32_t sid, const seasoninfo &info) {
        splayer_table table(self, self.value);
        table.emplace(self, [&](auto &target) {
            target.owner = from;
            target.powder = info.init_powder;
            target.current_stage = info.stage;
            target.last_rebirth = time_util::now_shifted() - 120;
        });
    }

    void ready_knight(name from, uint32_t sid) {
        sknight_table table(self, self.value);
        table.emplace(self, [&](auto &target) {
            target.owner = from;

            target.rows.push_back(knight_controller.new_knight(kt_knight));
            target.rows.push_back(knight_controller.new_knight(kt_archer));
            target.rows.push_back(knight_controller.new_knight(kt_mage));
        });
    }

    void remove_player(name from) {
        splayer_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter != table.cend()) {
            table.erase(iter);
        }
    }

    void remove_knight(name from) {
        sknight_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter != table.cend()) {
            table.erase(iter);
        }
    }

    void remove_item(name from) {
        sitem_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter != table.cend()) {
            table.erase(iter);
        }
    }

    void remove_material(name from) {
        smaterial_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter != table.cend()) {
            table.erase(iter);
        }
    }

    void remove_pet(name from) {
        spet_table table(self, self.value);
        auto iter = table.find(from.value);
        if (iter != table.cend()) {
            table.erase(iter);
        }
    }
};
