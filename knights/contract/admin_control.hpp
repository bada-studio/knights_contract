#pragma once

class admin_control : public control_base {
private:
    account_name self;
    adminstate_table adminvalues;
    stockholder_table stockholders;
    dividendlog_table dividendlogs;
    expenseslog_table expenseslogs;

public:
    const char* dividend_message = "dividend";

public:
    /// @brief
    /// Constructor
    admin_control(account_name _self)
            : self(_self)
            , adminvalues(_self, _self)
            , stockholders(_self, _self)
            , dividendlogs(_self, _self)
            , expenseslogs(_self, _self) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    name get_coo() {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "no admin values yet");
        return adminvalues.cbegin()->coo;
    }

    void record_new_player() {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "no admin values yet");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.player_count++;
        });
    }

    void add_tradingvol(const asset& quantity) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no user admin value");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.tradingvol += quantity;
        });
    }

    void add_revenue(name from, const asset& revenue, rv_type type) {
        if (revenue.amount == 0) {
            return;
        }

        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no user admin value");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue += revenue;
        });

        revenuedt2_table revenues(self, self);
        if (revenues.cbegin() == revenues.cend()) {
            revenues.emplace(self, [&](auto &target) {});
        }

        revenues.modify(revenues.begin(), self, [&](auto &target) {
            switch (type) {
                case rv_knight: target.knight += revenue; break;
                case rv_material_tax: target.material_tax += revenue; break;
                case rv_item_tax: target.item_tax += revenue; break;
                case rv_mp: target.mp += revenue; break;
                case rv_mat_iventory_up: target.mat_iventory_up += revenue; break;
                case rv_item_iventory_up: target.item_iventory_up += revenue; break;
                case rv_system: target.system += revenue; break;
                case rv_skin: target.coo_mat += revenue; break;
                case rv_dmw: target.dmw += revenue; break;
            }
        });

        add_wallet_revenue(from, revenue);
    }

    void add_wallet_revenue(name from, const asset &revenue) {
        if (from == to_name(self)) {
            return;
        }

        playerv2_table v2table(self, self);
        auto v2iter = v2table.find(from);
        if (v2iter == v2table.cend()) {
            return;
        }

        auto wallet = v2iter->wallet;
        revenuewt_table wtable(self, self);
        auto witer = wtable.find(wallet);
        if (witer == wtable.cend()) {
            return;
        }

        if (witer->waccount == 0) {
            return;
        }

        auto total = witer->revenue + revenue;
        auto deposit = total - witer->shared_for;
        auto threshold = 100000; // 10 EOS

        if (deposit.amount > threshold) {
            auto share_rate = 40; // it could be change
            auto share = deposit * 100 / share_rate;

            // share
            action(permission_level{self, N(active) },
                   N(eosio.token), N(transfer),
                   std::make_tuple(self, witer->waccount, share, std::string("share"))
            ).send();

            wtable.modify(witer, self, [&](auto &target) {
                target.revenue = total;
                target.shared_for = total;
                target.shared += share;
            });
        } else {
            wtable.modify(witer, self, [&](auto &target) {
                target.revenue = total;
            });
        }
    }

    void add_expenses(const asset& amount, name to, const std::string &memo) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no admin value");
        //assert_true(adminvalues.cbegin()->revenue.amount >= amount.amount, "overdrawn expenses");

        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue -= amount;
            target.expenses += amount;
        });

        auto no = expenseslogs.available_primary_key();
        auto current = time_util::now_shifted();
        expenseslogs.emplace(self, [&](auto &target) {
            target.no = no;
            target.at = current;
            target.amount = amount;
            target.to = to;
            target.memo = memo;
        });
    }

    void add_dividend(const asset& amount) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no admin value");
        assert_true(adminvalues.cbegin()->revenue.amount >= amount.amount, "overdrawn dividend");

        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue -= amount;
            target.dividend += amount;
        });
    }

    bool is_stock_holder(name owner) {
        auto sh = stockholders.find(owner);
        return sh != stockholders.cend();
    }

    void autodividend() {
        if (adminvalues.cbegin() == adminvalues.cend()) {
            return;
        }

        if (adminvalues.cbegin()->revenue.amount > 1100'0000) {
            asset amount(100'0000, S(4, EOS));
            dividend(amount);
        }
    }

    // actions
    //-------------------------------------------------------------------------
    /// @brief
    /// Set coo
    /// Coo can issue admin materials to market
    /// @param name
    /// coo account name
    void setcoo(name name) {
        require_auth(self);

        if (adminvalues.cbegin() == adminvalues.cend()) {
            adminvalues.emplace(self, [&](auto &target) {
                target.id = 0;
                target.pause = 0;
                target.coo = name;
            });
        } else {
            adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
                target.coo = name;
            });
        }
    }

    void setwifo(uint8_t id, name wname, name waccount) {
        require_auth(self);

        revenuewt_table wtable(self, self);
        auto witer = wtable.find(id);
        if (witer != wtable.cend()) {
            wtable.modify(witer, self, [&](auto &target) {
                target.wname = wname;
                target.waccount = waccount;
            });
        } else {
            wtable.emplace(self, [&](auto &target) {
                target.id = id;
                target.wname = wname;
                target.waccount = waccount;
            });
        }
    }

    /*    
    /// @brief
    /// Pause game
    /// User can not play this game until to pause valse is set to false.
    /// @param pause
    /// normal: 0, pause: 1
    void setpause(uint8_t pause) {
        require_auth(self);
        assert_true(pause >= 0 && pause <= 1, "invalid pause value");
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "no admin values");

        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.pause = pause;
        });
    }
    */

    /// @brief
    /// Register stock holder
    /// @param holder 
    /// Stock holder name
    /// @param share
    /// Stock share
    void regsholder(name holder, uint16_t share) {
        require_auth(self);

        auto sh = stockholders.find(holder);
        if (sh != stockholders.cend()) {
            if (share == 0) {
                stockholders.erase(sh);
            } else {
                stockholders.modify(sh, self, [&](auto &target) {
                    target.share = share;
                });
            }
        } else {
            stockholders.emplace(self, [&](auto &target) {
                target.holder = holder;
                target.share = share;
            });
        }
    }

    /// @brief
    /// Withdraw eos token to stockholders
    /// @param amount
    /// Amount of token to withdraw
    void dividend(asset amount) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "no revenue yet");

        std::vector<dividendto> logs;
        asset total(0, S(4, EOS));
        for (auto sh = stockholders.cbegin(); sh != stockholders.end(); ++sh) {
            asset price = amount * sh->share / 1000;
            total += price;
            action(permission_level{self, N(active) },
                   N(eosio.token), N(transfer),
                   std::make_tuple(self, sh->holder, price, std::string(dividend_message)) 
            ).send();

            dividendto log;
            log.to = sh->holder;
            log.amount = price;
            logs.push_back(log);
        }

        auto no = dividendlogs.available_primary_key();
        auto current = time_util::now_shifted();
        dividendlogs.emplace(self, [&](auto &target) {
            target.no = no;
            target.at = current;
            target.amount = total;
            target.to.insert(target.to.end(), logs.begin(), logs.end());
        });
    }

    void add_loss(const asset& revenue, const asset& loss) {
        require_auth(self);
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no user admin value");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue += revenue;
            target.revenue -= loss;
            target.loss += loss;
        });
    }
};
