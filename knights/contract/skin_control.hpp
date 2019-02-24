#pragma once

class skin_control : public control_base {
private:
    account_name self;

    player_control &player_controller;

public:
    // constructor
    //-------------------------------------------------------------------------
    /// @brief
    /// Constructor
    skin_control(account_name _self,
                player_control &_player_controller)
            : self(_self)
            , player_controller(_player_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------

    // actions
    //-------------------------------------------------------------------------
    void skissue(uint16_t code, uint32_t count, asset price) {
        require_auth(self);

        // get info
        uint32_t cid = 1;
        skininfo_table itable(self, self);
        auto iiter = itable.find(code);
        if (iiter != itable.cend()) {
            cid = iiter->count + 1;
            itable.modify(iiter, self, [&](auto &target) {
                target.count += count;
            });
        } else {
            itable.emplace(self, [&](auto &target) {
                target.code = code;
                target.count = count;
            });
        }

        // issue skin
        skin_table table(self, self);
        auto iter = table.find(self);
        if (iter != table.cend()) {
            table.modify(iter, self, [&](auto &target) {
                for (int index = 0; index < count; index++) {
                    skinrow row;
                    row.cid = cid + index;
                    row.code = code;
                    row.state = ss_selling;
                    target.rows.push_back(row);
                }
            });
        } else {
            table.emplace(self, [&](auto &target) {
                for (int index = 0; index < count; index++) {
                    skinrow row;
                    row.cid = cid + index;
                    row.code = code;
                    row.state = ss_selling;
                    target.owner = to_name(self);
                    target.rows.push_back(row);
                }
            });
        }

        skin4sale_table mtable(self, self);
        auto miter = mtable.find(code);
        if (miter != mtable.cend()) {
            mtable.modify(miter, self, [&](auto &target) {
                for (int index = 0; index < count; index++) {
                    skin4salerow row;
                    row.mid = ++target.last_mid;
                    row.cid = cid + index;
                    row.code = code;
                    row.seller = to_name(self);
                    row.price = price;
                    target.rows.push_back(row);
                }
            });
        } else {
            mtable.emplace(self, [&](auto &target) {
                target.code = code;
                target.last_mid = 0;
                for (int index = 0; index < count; index++) {
                    skin4salerow row;
                    row.mid = ++target.last_mid;
                    row.cid = cid + index;
                    row.code = code;
                    row.seller = to_name(self);
                    row.price = price;
                    target.rows.push_back(row);
                }
            });
        }
    }

    void sksell(name from, uint64_t cid, uint16_t code, asset price) {
        skin_table table(self, self);
        auto iter = table.find(self);
        auto pos = iter->get_skin(cid);
        assert_true(pos >= 0, "can not found skin");

        auto &skin = iter->rows[pos];
        assert_true(skin.state != ss_selling, "already on sale");

        table.modify(iter, self, [&](auto &target) {
            target.rows[pos].state = ss_selling;
        });

        skin4sale_table mtable(self, self);
        auto miter = mtable.find(code);
        if (miter != mtable.cend()) {
            auto mpos = miter->get_skin_by_cid(cid);
            assert_true(mpos == -1, "already on sale");
            
            mtable.modify(miter, self, [&](auto &target) {
                skin4salerow row;
                row.mid = ++target.last_mid;
                row.cid = cid;
                row.code = code;
                row.seller = from;
                row.price = price;
                target.rows.push_back(row);
            });
        } else {
            mtable.emplace(self, [&](auto &target) {
                skin4salerow row;
                row.mid = ++target.last_mid;
                row.cid = cid;
                row.code = code;
                row.seller = from;
                row.price = price;
                target.code = code;
                target.rows.push_back(row);
            });
        }
    }

    void skcsell(name from, uint16_t code, uint64_t mid) {
        skin4sale_table mtable(self, self);
        auto miter = mtable.find(code);
        assert_true(miter != mtable.cend(), "can not found skin");

        auto mpos = miter->get_skin_by_mid(mid);
        assert_true(mpos >= 0, "can not found skin");

        mtable.emplace(self, [&](auto &target) {
            target.rows.erase(miter->rows.begin() + mpos);
        });
    }

    asset skbuy(name from, const transfer_action &ad) {
        require_auth(from);

        // find skin in market 
        skin4sale_table mtable(self, self);
        auto miter = mtable.find(ad.type);
        assert_true(miter != mtable.cend(), "can not found skin");

        uint64_t mid = atoll(ad.param.c_str());
        auto mpos = miter->get_skin_by_mid(mid);
        assert_true(mpos >= 0, "can not found skin");

        auto &mskin = miter->rows[mpos];
        assert_true(ad.quantity == mskin.price, "price not matching");

        mtable.modify(miter, self, [&](auto &target) {
            target.rows.erase(miter->rows.begin() + mpos);
        });

        // move skin
        skin_table table(self, self);
        auto iter = table.find(mskin.seller);
        assert_true(iter != table.cend(), "can not found skin");

        auto pos = iter->get_skin(mskin.cid);
        assert_true(pos >= 0, "can not found skin");

        auto &skin = iter->rows[pos];
        assert_true(skin.state == ss_selling, "not on sale");

        // remove from seller
        table.modify(iter, self, [&](auto &target) {
            target.rows.erase(target.rows.begin() + pos);
        });

        // add to buyer
        auto iter2 = table.find(from);
        if (iter2 != table.cend()) {
            int pos2 = iter2->get_skin_by_code(mskin.code);
            assert_true(pos2 < 0, "already have same skin");

            table.modify(iter2, self, [&](auto &target) {
                skinrow row;
                row.cid = mskin.cid;
                row.code = mskin.code;
                row.state = ss_normal;
                target.rows.push_back(row);
            });
        } else {
            table.emplace(self, [&](auto &target) {
                skinrow row;
                row.cid = mskin.cid;
                row.code = mskin.code;
                row.state = ss_normal;
                target.owner = from;
                target.rows.push_back(row);
            });
        }

        // calcuate tax
        int tax_rate = kv_market_tax_rate;
        if (mskin.seller == self) {
            tax_rate = 0;
        }

        asset tax(0, S(4, EOS));
        asset price = mskin.price;
        if (tax_rate > 0) {
            tax = price * tax_rate / 100;
            if (tax.amount == 0) {
                tax.amount = 1;
            }
            price -= tax;
        }

        // send token
        if (mskin.seller != self) {
            auto message = std::string("eosknights:skin-sale:") + 
                        std::to_string(mskin.code) + ":" + 
                        std::to_string(mskin.cid) + ":" + 
                        from.to_string();
            action(permission_level{ self, N(active) },
                N(eosio.token), N(transfer),
                std::make_tuple(self, mskin.seller, price, message)
            ).send();
        }
        
        return tax;
    }

    void skwear(name from, uint32_t knt, uint32_t cid) {
        skin_table table(self, self);
        auto iter = table.find(from);
        assert_true(iter != table.cend(), "can not found skin");

        int pos = -1;
        if (cid > 0) {
            pos = iter->get_skin(cid);
            assert_true(pos >= 0, "can not found skin");

            auto &skin = iter->rows[pos];
            assert_true(skin.state == ss_normal, "available only when normal");
        }

        table.modify(iter, self, [&](auto &target) {
            for (int index = 0; index < target.rows.size(); index++) {
                int current_knt = target.rows[index].code / 10;
                if (current_knt == knt && target.rows[index].state == ss_wear) {
                    target.rows[index].state = ss_normal;
                }
            }

            if (cid > 0) {
                target.rows[pos].state = ss_wear;
            }
        });        
    }
};
