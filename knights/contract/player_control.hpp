#pragma once

class player_control : public control_base {
private:
    player_table players;
    playerv_table playervs;

    account_name self;
    rule_controller<rivnprice, rivnprice_table> rivnprice_controller;
    saleslog_control &saleslog_controller;
    admin_control &admin_controller;
    variable_control &variable_controller;

    struct st_transfer {
        account_name from;
        account_name to;
        asset        quantity;
        std::string  memo;
    };
    
public:
    // constructor
    //-------------------------------------------------------------------------
    player_control(account_name _self,
                   saleslog_control &_saleslog_controller,
                   admin_control &_admin_controller,
                   variable_control &_variable_controller)
        : self(_self)
        , players(_self, _self)
        , playervs(_self, _self)
        , rivnprice_controller(_self, N(ivnprice))
        , saleslog_controller(_saleslog_controller)
        , admin_controller(_admin_controller)
        , variable_controller(_variable_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    player_table& get_players() {
        return players;
    }

    player_table::const_iterator get_player(name player_name) {
        return players.find(player_name);
    }
    
    bool is_empty_player(player_table::const_iterator player) {
        return player == players.end();
    }

    void increase_powder(player_table::const_iterator player, uint32_t powder) {
        // modify powder
        players.modify(player, self, [&](auto& target) {
            target.powder += powder;
        });
    }

    void decrease_powder(player_table::const_iterator player, uint32_t powder) {
        assert_true(player->powder >= powder, "not enough powder");

        // modify powder
        players.modify(player, self, [&](auto& target) {
            target.powder -= powder;
        });
    }

    template<typename T>
    void eosiotoken_transfer(uint64_t sender, uint64_t receiver, T func) {
        auto transfer_data = eosio::unpack_action_data<st_transfer>();
        eosio_assert(transfer_data.quantity.symbol == S(4, EOS), "only accepts EOS for deposits");
        eosio_assert(transfer_data.quantity.is_valid(), "Invalid token transfer");
        eosio_assert(transfer_data.quantity.amount > 0, "Quantity must be positive");

        if (transfer_data.from == self) {
            auto to = to_name(transfer_data.to);
            auto to_player = players.find(to);
            check_blacklist(to);

            // stockholder could be stockholder. so separate withdraw and dividend by message
            if (admin_controller.is_stock_holder(to) && 
                transfer_data.memo.compare(admin_controller.dividend_message) == 0) {
                // stock share
                admin_controller.add_dividend(transfer_data.quantity);
            } else if (to_player != players.cend()) {
                // player withdraw
                return;
            } else {
                // expense
                admin_controller.add_expenses(transfer_data.quantity, to, transfer_data.memo);
            }
        } else if (transfer_data.to == self) {
            auto from = to_name(transfer_data.from);
            auto from_player = players.find(from);
            check_blacklist(from);
            
            if (transfer_data.memo == "investment") {
                admin_controller.add_investment(transfer_data.quantity);
            } else if (from_player == players.end()) {
                // system account could transfer eos to contract
                // eg) unstake, sellram, etc
                // add to the revenue for these.
                if (is_system_account(transfer_data.from)) {
                    admin_controller.add_revenue(transfer_data.quantity, rv_system);
                } else {
                    assert_true(false, "sign up first!");
                }
            } else {
                // player's deposit action
                transfer_action res;
                size_t center = transfer_data.memo.find(':');
                size_t next = transfer_data.memo.find(':', center + 1);
                res.from = from;
                res.action = transfer_data.memo.substr(0, center);

                if (next != std::string::npos) {
                    res.param = transfer_data.memo.substr(center + 1);
                } else {
                    // param:type:seller;
                    res.param = transfer_data.memo.substr(center + 1, next - (center + 1));
                    size_t next2 = transfer_data.memo.find(':', next + 1);
                    size_t next3 = transfer_data.memo.find(':', next2 + 1);
                    res.type = atoi(transfer_data.memo.substr(next2 + 1, next3 - (next2 + 1)).c_str());
                    res.seller = to_name(atoll(transfer_data.memo.substr(next3 + 1).c_str()));
                }

                res.quantity = transfer_data.quantity;       
                func(res);
            }
        }
    }

    bool is_system_account(account_name name) {
        if (name == N(eosio.bpay) || 
            name == N(eosio.msig) ||
            name == N(eosio.names) ||
            name == N(eosio.ram) ||
            name == N(eosio.ramfee) ||
            name == N(eosio.saving) ||
            name == N(eosio.stake) || 
            name == N(eosio.token) || 
            name == N(eosio.vpay) ) {
            return true;
        }
        return false;
    }

    void new_player(name from) {
        auto& variables = variable_controller.get_rvariable_rule();
        auto& rules = variables.get_table();
        auto rule = rules.find(vt_init_powder);
        eosio_assert(rule != rules.end(), "can not found powder rule" );

        auto itr = players.emplace(self, [&](auto& target) {
            target.owner = from;
            target.powder = rule->value;
            target.current_stage = 1;
        });

        admin_controller.record_new_player();
    }

    uint32_t seed_identity(name from);
    random_val begin_random(name from, random_for r4, int type);
    void end_random(name from, const random_val &val, random_for r4, int type);
    uint32_t get_key(name from);
    uint32_t get_checksum_key(name from);

    void new_playervs(name from, int8_t referral, int16_t gift) {
        playervs.emplace(self, [&](auto& target) {
            target.owner = from;
            target.referral = referral;
            target.gift = gift;
        });
    }

    void checksum_gateway(name from, uint32_t block, uint32_t checksum) {
        int32_t v1 = (checksum >> 16);
        if (v1 & 0x8000) {
            test_checksum_v2(from, block, checksum);
        } else {
            test_checksum(((int64_t)block << 32) | checksum);
        }
    }
    
    int test_checksum(uint64_t checksum) {
        int32_t k = (checksum_mask & 0xFFFF);
        int32_t num = tapos_block_num();

        int64_t v1 = (checksum >> 32);
        int32_t v2 = get_checksum_value((checksum >> 16) & 0xFFFF);
        int32_t v3 = get_checksum_value((checksum) & 0xFFFF);
        assert_true((v1 % k) == v3, "checksum failure");
        assert_true((num - v1) < 120, "too old checksum");
        return (int)(num + v1 + v2 + v3) % k;
    }

    void test_checksum_v2(name from, uint32_t block, uint32_t checksum) {
        int32_t k = (checksum_mask & 0xFFFF);
        int32_t num = time_util::getnow();

        int32_t v0 = block ^ get_checksum_key(from);
        int32_t v1 = (v0 % k);
        int32_t v2 = (checksum >> 16) & 0x7FFF;
        int32_t v3 = get_checksum_value((checksum) & 0xFFFF);
        assert_true(v1 == v2, "checksum failure 1");
        assert_true(v2 == v3, "checksum failure 2");
        assert_true((num + 60) > v0, "checksum failure 3");
        assert_true((num - v0) < 90, "too old action");
    }

    void require_action_count(int count) {
        char buffer[512];
        int actual_size = read_transaction(buffer, 512);
        eosio::datastream<const char *> ds(buffer, actual_size);
        eosio::transaction tx;
        ds >> tx;
        eosio_assert((tx.actions.end() - tx.actions.begin()) == count, "wrong number of actions in transaction");
        eosio_assert(tx.actions[count-1].account == self, "wrong action recipient"); 
    }

    int32_t get_checksum_value(int32_t value) {
        uint64_t a = checksum_mask >> 16;
        uint64_t b = checksum_mask & 0xFFFF;
        uint64_t c = value;
        uint64_t d = 1;
        for (int index = 0; index < a; index++) {
            d *= c;
            d %= b;
        }

        return d;
    }

    void check_blacklist(name from) {
        if (from == N(valuenetwork) || 
            from == N(ramcollector) || 
            from == N(mrnumberzero) || 
            from == N(siuhangmeiyu) || 
            from == N(amazinggamer) || 
            from == N(mantikmantik) || 
            from == N(meiyusiuhang) ||
            from == N(gameplayer11) ||
            from == N(gameplayer12) ||
            from == N(gameplayer13) ||
            from == N(gameplayer14) ||
            from == N(gameplayer15) ||
            from == N(eos4chatting) ||
            from == N(gi4temzqhege) ||
            from == N(messcomposer) ||
            from == N(pvtmessenger) ||
            from == N(eospromoter1)
        ) {
            assert_true(false, "blacklist rejected");
        }
    }

    void require_coo_auth() {
        if (has_auth(self)) {
            return;
        }

        return require_auth(admin_controller.get_coo());
    }

    // actions
    //-------------------------------------------------------------------------
    void signup(name from) {
        require_auth(from);
        auto iter = players.find(from);
        eosio_assert(iter == players.end(), "already signed up" );
        new_player(from);
    }

    void referral(name from, name to) {
        require_auth(from);

        assert_true(from != to, "wrong recipient");

        auto fplayer = players.find(from);
        auto tplayer = players.find(to);
        assert_true(fplayer != players.end(), "could not find player");
        assert_true(tplayer != players.end(), "could not find player");

        auto fplayerv = playervs.find(from);
        auto tplayerv = playervs.find(to);
        assert_true(fplayer->last_rebirth > 0, "one or more knight required.");
        assert_true(tplayer->last_rebirth > 0, "one or more knight required for the recipient.");

        if (fplayerv == playervs.cend()) {
            uint8_t referral = 0x80;
            referral++;
            new_playervs(from, referral, 0);
        } else {
            int referral = fplayerv->referral;
            int count = get_referral_count(referral);
            assert_true((referral & 0x80) == 0, "you have already received a referral bonus.");
            assert_true(count < kv_referral_max, "you received already maximum referral bonus");
            playervs.modify(fplayerv, self, [&](auto& target) {
                target.referral |= 0x80;
                target.referral++;
            });
        }

        if (tplayerv == playervs.cend()) {
            new_playervs(to, 1, 0);
        } else {
            int referral = tplayerv->referral;
            int count = get_referral_count(referral);
            assert_true(count < kv_referral_max, "recipient received already maximum referral bonus");
            playervs.modify(tplayerv, self, [&](auto& target) {
                target.referral++;
            });
        }

        players.modify(fplayer, self, [&](auto& target) {
            target.powder += kv_referral_bonus;
        });

        players.modify(tplayer, self, [&](auto& target) {
            target.powder += kv_referral_bonus;
        });
    }

    void addgift(uint16_t no, uint8_t type, uint16_t amount, uint32_t to) {
        require_auth(self);

        gift_table gifts(self, self);
        if (gifts.begin() == gifts.cend()) {
            gifts.emplace(self, [&](auto& target) {
                target.key = 1;
                target.no = no;
                target.type = type;
                target.amount = amount;
                target.to = to - time_util::origin;
            });
        } else {
            gifts.modify(gifts.begin(), self, [&](auto& target) {
                target.key = 1;
                target.no = no;
                target.type = type;
                target.amount = amount;
                target.to = to - time_util::origin;
            });
        }
    }

    void getgift(name from, int16_t no) {
        require_auth(from);

        auto current = time_util::getnow();
        gift_table gifts(self, self);
        assert_true(gifts.cbegin() != gifts.cend(), "invalid gift");
        assert_true(gifts.cbegin()->no == no, "invalid gift");
        assert_true(gifts.cbegin()->to >= current, "the gift is over");

        auto playerv = playervs.find(from);
        if (playerv == playervs.cend()) {
            new_playervs(from, 0, no);
        } else {
            assert_true(playerv->gift < no, "you already got gift");
            playervs.modify(playerv, self, [&](auto& target) {
                target.gift = no;
            });
        }

        auto amount = gifts.cbegin()->amount;
        assert_true(amount > 0, "no gift");

        auto player = get_player(from);
        increase_powder(player, amount);
    }

    int32_t get_referral_count(uint8_t referral) {
        return 0x7F & referral;
    }

    void itemivnup(name from, const asset &quantity) {
        require_auth(from);
        auto player = players.find(from);
        assert_true(player != players.end(), "could not find player");


        uint8_t ts = player->item_ivn_up + 1;
        assert_true(ts <= kv_max_item_inventory_up, "can not exceed max size");

        auto &ivnprice_table = rivnprice_controller.get_table();
        auto price = ivnprice_table.find((uint64_t)ts);
        assert_true(price != ivnprice_table.end(), "no price rule");
        assert_true(quantity.amount == price->price.amount, "ivn price does not match");

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::getnow();
        blog.type = ct_item_iventory_up;
        blog.pid = ts;
        blog.code = 0;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = price->price;
        saleslog_controller.add_buylog(blog, from);

        // modify inventory size
        players.modify(player, self, [&](auto& target) {
            target.item_ivn_up = ts;
        });
    }

    void mativnup(name from, const asset &quantity) {
        require_auth(from);
        auto player = players.find(from);
        assert_true(player != players.end(), "could not find player");

        uint8_t ts = player->mat_ivn_up + 1;
        assert_true(ts <= kv_max_material_inventory_up, "can not exceed max size");

        auto &ivnprice_table = rivnprice_controller.get_table();
        auto price = ivnprice_table.find((uint64_t)ts);
        assert_true(price != ivnprice_table.end(), "no price rule");
        assert_true(quantity.amount == price->price.amount, "ivn price does not match");

        name seller;
        seller.value = self;

        buylog blog;
        blog.seller = seller;
        blog.dt = time_util::getnow();
        blog.type = ct_mat_iventory_up;
        blog.pid = ts;
        blog.code = 0;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = price->price;
        saleslog_controller.add_buylog(blog, from);

        // modify inventory size
        players.modify(player, self, [&](auto& target) {
            target.mat_ivn_up = ts;
        });
    }

    void shuffle(name from) {
        require_auth(self);

        auto rval = begin_random(from, r4_rebirth, 0);
        rval.seed = seed_identity(from);
        end_random(from, rval, r4_rebirth, 0);
        
        rval.range(10);
        end_random(from, rval, r4_petgacha, pgt_low_class);
        
        rval.range(10);
        end_random(from, rval, r4_petgacha, pgt_high_class);

        rval.range(10);
        end_random(from, rval, r4_craft, ig_normal);

        rval.range(10);
        end_random(from, rval, r4_craft, ig_rare);

        rval.range(10);
        end_random(from, rval, r4_craft, ig_unique);

        rval.range(10);
        end_random(from, rval, r4_craft, ig_legendary);

        rval.range(10);
        end_random(from, rval, r4_craft, ig_ancient);

        rval.range(10);
        end_random(from, rval, r4_petexp, 0);
    }

    rule_controller<rivnprice, rivnprice_table>& get_inventory_price_rule() {
        return  rivnprice_controller;
    }
};
