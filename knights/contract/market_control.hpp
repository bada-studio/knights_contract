#pragma once

class market_control : public control_base {
private:
    account_name self;
    item4sale_table items;
    mat4sale_table materials;

    player_control &player_controller;
    item_control &item_controller;
    material_control &material_controller;
    saleslog_control &saleslog_controller;
    knight_control &knight_controller;

    // modifiers
    //-------------------------------------------------------------------------
    void validate_price(asset price, int grade) {
        assert_true(price.symbol == S(4,EOS) , "only EOS token allowed");
        assert_true(price.is_valid(), "invalid price");
        assert_true(price.amount > 0, "must price positive quantity");

        assert_true(price.amount >= get_min_market_price(grade), "too small price");
        assert_true(price.amount <= kv_max_market_price, "too big price");
    }

    int get_min_market_price(int grade) {
        int price = kv_min_market_price;
        int scaler = kv_min_market_price_scaler;
        
        if (grade >= ig_rare) {
            price *= (scaler & 0xF);
        } 
        if (grade >= ig_unique) {
            price *= ((scaler >> 4) & 0xF);
        } 
        if (grade >= ig_legendary) {
            price *= ((scaler >> 8) & 0xF);
        } 
        if (grade >= ig_ancient) {
            price *= ((scaler >> 12) & 0xF);
        }

        return price;
    }

    uint64_t next_pid(marketpid_type type) {
        marketpid_table table(self, self);
        auto iter = table.find(type);
        uint64_t pid = 1;
        
        if (iter != table.cend()) {
            pid = iter->pid + 1;
            table.modify(iter, self, [&](auto &target) {
               target.pid = pid; 
            });
        } else {
            table.emplace(self, [&](auto &target) {
               target.type = type;
               target.pid = pid; 
            });
        }

        return pid;
    }

public:
    // constructor
    //-------------------------------------------------------------------------
    market_control(account_name _self,
                   material_control &_material_controller,
                   item_control &_item_controller,
                   player_control &_player_controller,
                   saleslog_control &_saleslog_controller,
                   knight_control &_knight_controller)
            : self(_self)
            , items(_self, _self)
            , materials(_self, _self)
            , material_controller(_material_controller)
            , item_controller(_item_controller)
            , player_controller(_player_controller)
            , saleslog_controller(_saleslog_controller)
            , knight_controller(_knight_controller) {
    }

    // internal apis
    //-------------------------------------------------------------------------
    uint64_t issue_mat(uint16_t code, asset price, name from) {
        uint64_t id = next_pid(mpidt_material);
        if (id == 0) {
            id = 1;
        }

        auto &rule_table = material_controller.get_rmaterial_rule().get_table();
        auto rule = rule_table.find(code);
        assert_true(rule != rule_table.cend(), "could not find material rule");

        materials.emplace(self, [&](auto& target) {
            target.cid = id;
            target.price = price;
            target.code = code;
            target.player = from;
        });

        return id;
    }

    // action
    //-------------------------------------------------------------------------
    void sellitem(name from, uint64_t itemid, asset price) {
        require_auth(from);
        player_controller.check_blacklist(from);

        auto &rows = item_controller.get_items(from);
        auto &item = item_controller.get_item(rows, itemid);
        assert_true(item.saleid == 0, "already on sale");
        assert_true(item.knight == 0, "equipped item can not be sold");

        auto &item_rules = item_controller.get_ritem_rule().get_table();
        auto item_rule = item_rules.find(item.code);
        assert_true(item_rule != item_rules.cend(), "can not found item grade");
        validate_price(price, item_rule->grade);

        int sale_count = 0;
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].saleid > 0) {
                sale_count++;
            }
        }

        auto &knights = knight_controller.get_knights(from);
        auto max_sell_count = get_max_sell_count(from);
        assert_true(sale_count < max_sell_count, "sell limit");

        uint64_t id = next_pid(mpidt_item);
        if (id == 0) {
            id = 1;
        }

        items.emplace(self, [&](auto& target) {
            target.cid = id;
            target.price = price;
            target.code = item.code;
            target.dna = item.dna;
            target.level = item.level;
            target.exp = item.exp;
            target.player = from;
        });

        item_controller.make_item_forsale(from, itemid, id);
    }

    void ccsellitem(name from, uint64_t saleid) {
        require_auth(from);
        auto saleitem = items.find(saleid);
        assert_true(saleitem != items.end(), "could not find item");

        item_controller.cancel_sale(from, saleid);
        items.erase(saleitem);
    }

    asset buyitem(name from, const transfer_action &ad) {
        require_auth(from);

        uint64_t saleid = atoll(ad.param.c_str());
        const asset &quantity = ad.quantity;

        auto saleitem = items.find(saleid);
        assert_true(saleitem != items.end(), "could not find item");
        assert_true(saleitem->player != from, "it's your item");
        if (ad.seller.value > 0) {
            assert_true(saleitem->player == ad.seller, "seller not matching");
        }

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "can not found player info");

        // inventory check
        int max_inventory_size = item_controller.get_max_inventory_size(*player);
        int current_inventory_size = item_controller.get_items(from).size();
        assert_true(current_inventory_size < max_inventory_size, "full inventory");

        if (saleitem->player != self) {
            item_controller.remove_saleitem(saleitem->player, saleitem->cid);
        }

        item_controller.new_item_from_market(from, saleitem->code, saleitem->dna, saleitem->level, saleitem->exp);
        items.erase(saleitem);
        int tax_rate = kv_market_tax_rate;
        if (saleitem->player == self) {
            tax_rate = 0;
        }

        asset price = saleitem->price;
        assert_true(quantity.amount == price.amount, "item price does not match");
        auto dt = time_util::getnow();

        selllog slog;
        slog.buyer = from;
        slog.dt = dt;
        slog.type = ct_item;
        slog.pid = saleitem->cid;
        slog.code = saleitem->code;
        slog.dna = saleitem->dna;
        slog.level = saleitem->level;
        slog.exp = saleitem->exp;
        slog.price = saleitem->price;
        slog.taxrate = tax_rate;
        saleslog_controller.add_saleslog(slog, saleitem->player);

        buylog blog;
        blog.seller = saleitem->player;
        blog.dt = dt;
        blog.type = ct_item;
        blog.pid = saleitem->cid;
        blog.code = saleitem->code;
        blog.dna = saleitem->dna;
        blog.level = saleitem->level;
        blog.exp = saleitem->exp;
        blog.price = saleitem->price;
        saleslog_controller.add_buylog(blog, from);

        // calculate tax
        asset tax(0, S(4, EOS));
        if (tax_rate > 0) {
            tax = price * tax_rate / 100;
            if (tax.amount == 0) {
                tax.amount = 1;
            }
            price -= tax;
        }

        auto message = std::string("eosknights:item-sale:") + 
                       std::to_string(saleitem->code) + ":" + 
                       std::to_string(saleitem->cid) + ":" + 
                       from.to_string();
        action(permission_level{ self, N(active) },
               N(eosio.token), N(transfer),
               std::make_tuple(self, saleitem->player, price, message)
        ).send();

        return tax;
    }

    void sellmat(name from, uint64_t matid, asset price) {
        require_auth(from);
        player_controller.check_blacklist(from);

        auto &rows = material_controller.get_materials(from);
        auto &mat = material_controller.get_material(rows, matid);
        assert_true(mat.saleid == 0, "already on sale");

        auto &mat_rules = material_controller.get_rmaterial_rule().get_table();
        auto mat_rule = mat_rules.find(mat.code);
        assert_true(mat_rule != mat_rules.cend(), "can not found material grade");
        validate_price(price, mat_rule->grade);

        int sale_count = 0;
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].saleid > 0) {
                sale_count++;
            }
        }

        auto &knights = knight_controller.get_knights(from);
        auto max_sell_count = get_max_sell_count(from);
        assert_true(sale_count < max_sell_count, "sell limit");

        uint64_t id = issue_mat(mat.code, price, from);
        material_controller.make_material_forsale(from, matid, id);
    }

    void ccsellmat(name from, uint64_t saleid) {
        require_auth(from);
        auto salemat = materials.find(saleid);
        assert_true(salemat != materials.end(), "could not find mat");

        material_controller.cancel_sale(from, saleid);
        materials.erase(salemat);
    }

    asset buymat(name from, const transfer_action &ad) {
        require_auth(from);

        uint64_t saleid = atoll(ad.param.c_str());
        const asset &quantity = ad.quantity;

        auto &players = player_controller.get_players();
        auto player = players.find(from);
        assert_true(players.cend() != player, "can not found player info");

        // inventory check
        int max_inventory_size = material_controller.get_max_inventory_size(*player);
        int current_inventory_size = material_controller.get_materials(from).size();
        assert_true(current_inventory_size < max_inventory_size, "full inventory");

        auto salemat = materials.find(saleid);
        assert_true(salemat != materials.end(), "could not find mat");
        assert_true(salemat->player != from, "it's your mat");
        if (ad.seller.value > 0) {
            assert_true(salemat->player == ad.seller, "seller not matching");
        }

        material_controller.remove_salematerial(salemat->player, salemat->cid);
        material_controller.new_material_from_market(from, salemat->code);
        materials.erase(salemat);

        int tax_rate = kv_market_tax_rate;
        if (salemat->player == self) {
            tax_rate = 0;
        }

        asset price = salemat->price;
        assert_true(quantity.amount == price.amount, "material price does not match");
        
        auto dt = time_util::getnow();

        selllog slog;
        slog.buyer = from;
        slog.dt = dt;
        slog.type = ct_material;
        slog.pid = salemat->cid;
        slog.code = salemat->code;
        slog.dna = 0;
        slog.level = 0;
        slog.exp = 0;
        slog.price = salemat->price;
        slog.taxrate = tax_rate;
        saleslog_controller.add_saleslog(slog, salemat->player);
        
        buylog blog;
        blog.seller = salemat->player;
        blog.dt = dt;
        blog.type = ct_material;
        blog.pid = salemat->cid;
        blog.code = salemat->code;
        blog.dna = 0;
        blog.level = 0;
        blog.exp = 0;
        blog.price = salemat->price;
        saleslog_controller.add_buylog(blog, from);

        // calculate tax
        asset tax(0, S(4, EOS));
        if (tax_rate > 0) {
            tax = price * tax_rate / 100;
            if (tax.amount == 0) {
                tax.amount = 1;
            }
            price -= tax;
        }

        auto message = std::string("eosknights:material-sale:") + 
                       std::to_string(salemat->code) + ":" + 
                       std::to_string(salemat->cid) + ":" + 
                       from.to_string();

        action(permission_level{ self, N(active) },
               N(eosio.token), N(transfer),
               std::make_tuple(self, salemat->player, price, message)
        ).send();

        return tax;
    }

private:
    int get_max_sell_count(name from) {
        auto &knights = knight_controller.get_knights(from);
        auto max_sell_count = knights.size();
        auto player = player_controller.get_player(from);
        if (player->maxfloor >= kv_bonus_sell1_floor) {
            max_sell_count++;
        }
        if (player->maxfloor >= kv_bonus_sell2_floor) {
            max_sell_count++;
        }

        return max_sell_count;
    }
};