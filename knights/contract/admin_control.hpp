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
    bool is_paused() {
        if (adminvalues.cbegin() == adminvalues.cend()) {
            return false;
        }

        return adminvalues.cbegin()->pause == 1;
    }

    void entry_check(name name) {
        if (name == self) {
            return;
        }

        assert_true(!is_paused(), "game is paused");
    }

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

    void add_investment(const asset& quantity) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "no admin values yet");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.investment += quantity;
        });
    }

    void add_tradingvol(const asset& quantity) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no user admin value");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.tradingvol += quantity;
        });
    }

    void add_revenue(const asset& revenue, rv_type type) {
        if (revenue.amount == 0) {
            return;
        }

        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no user admin value");
        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue += revenue;
        });

        revenuedt_table revenues(self, self);
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
                case rv_coo_mat: target.coo_mat += revenue; break;
                case rv_system: target.system += revenue; break;
            }
        });
    }

    void add_expenses(const asset& amount, name to, const std::string &memo) {
        assert_true(adminvalues.cbegin() != adminvalues.cend(), "there is no admin value");
        //assert_true(adminvalues.cbegin()->revenue.amount >= amount.amount, "overdrawn expenses");

        adminvalues.modify(adminvalues.cbegin(), self, [&](auto &target) {
            target.revenue -= amount;
            target.expenses += amount;
        });

        auto no = expenseslogs.available_primary_key();
        auto current = time_util::getnow();
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
        require_auth(self);
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
        auto current = time_util::getnow();
        dividendlogs.emplace(self, [&](auto &target) {
            target.no = no;
            target.at = current;
            target.amount = total;
            target.to.insert(target.to.end(), logs.begin(), logs.end());
        });
    }
};
