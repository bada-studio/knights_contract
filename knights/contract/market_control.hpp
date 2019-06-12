#pragma once

class market_control : public control_base {
private:
    name self;
    item4sale_table items;
    mat4sale_table materials;

    system_control &system_controller;
    player_control &player_controller;
    item_control &item_controller;
    material_control &material_controller;
    saleslog_control &saleslog_controller;
    knight_control &knight_controller;

    // modifiers
    //-------------------------------------------------------------------------
    uint64_t next_pid(marketpid_type type) {
        marketpid_table table(self, self.value);
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
    market_control(name _self,
                   system_control &_system_controller,
                   player_control &_player_controller,
                   material_control &_material_controller,
                   item_control &_item_controller,
                   saleslog_control &_saleslog_controller,
                   knight_control &_knight_controller)
            : self(_self)
            , items(_self, _self.value)
            , materials(_self, _self.value)
            , system_controller(_system_controller)
            , player_controller(_player_controller)
            , material_controller(_material_controller)
            , item_controller(_item_controller)
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

        rmaterial_table rule_table(self, self.value);
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
        system_controller.check_blacklist(from);
        require_sell_cooltime(from);

        auto &rows = item_controller.get_items(from);
        auto &item = item_controller.get_item(rows, itemid);
        assert_true(item.saleid == 0, "already on sale");
        assert_true(item.knight == 0, "equipped item can not be sold");

        ritem_table item_rules(self, self.value);
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
        assert_true(knights.size() > 0, "there is no knight");

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
        set_sell_factor(from);
    }

    void require_sell_cooltime(name from) {
        auto pvsi = system_controller.get_playervs(from);
        uint32_t next = pvsi->last_sell_time + (int)(10 * (pvsi->sell_factor / 100.0));
        int32_t current = time_util::now_shifted();
        assert_true(current >= next, "too short to sell");
    }

    void set_sell_factor(name from) {
        auto pvsi = system_controller.get_playervs(from);
        auto variable = *pvsi;

        int32_t current = time_util::now_shifted();
        uint32_t last_sell_time = variable.last_sell_time;
        double sell_factor = variable.sell_factor / 100.0;
        sell_factor = std::max(1.0, sell_factor);
        sell_factor = std::min(6.0, sell_factor);

        int threshold = time_util::min;
        auto past = current - last_sell_time;
        
        if (past < threshold) {
            double rate = 1 + (threshold - past) / (double)threshold;
            sell_factor *= rate;
        } else {
            double rate = 1 + (past - threshold) / (double)threshold;
            sell_factor /= rate;
        }

        sell_factor = std::max(1.0, sell_factor);
        sell_factor = std::min(6.0, sell_factor);
        variable.sell_factor = (int)(sell_factor * 100);
        variable.last_sell_time = current;
        system_controller.update_playerv(pvsi, variable);
    }

    asset buyitem(name from, const transfer_action &ad, item_control_actions *ctrl, int max_grade) {
        require_auth(from);
        system_controller.checksum_gateway(from, ad.block, ad.checksum);

        uint64_t saleid = atoll(ad.param.c_str());
        const asset &quantity = ad.quantity;

        auto saleitem = items.find(saleid);
        assert_true(saleitem != items.end(), "could not find item");
        assert_true(saleitem->player != from, "it's your item");
        if (ad.seller.value > 0) {
            //assert_true(saleitem->player == ad.seller, "seller not matching");
        }

        // grade check
        assert_true(max_grade > ig_none, "can not buy item this mode");
        if (max_grade != ig_count) {
            ritem_table rule_table(self, self.value);
            auto rule = rule_table.find(saleitem->code);
            assert_true(rule != rule_table.cend(), "could not find material rule");
            assert_true(rule->grade <= max_grade, "can not buy this item");
        }

        auto &players = player_controller.get_players();
        auto player = players.find(from.value);
        assert_true(players.cend() != player, "can not found player info");

        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() > 0, "there is no knight");

        // inventory check
        int max_inventory_size = ctrl->get_max_inventory_size(player->item_ivn_up);
        int current_inventory_size = ctrl->get_items(from).size();
        assert_true(current_inventory_size < max_inventory_size, "full inventory");

        if (saleitem->player != self) {
            // remove from normal mode
            item_controller.remove_saleitem(saleitem->player, saleitem->cid);
        }

        ctrl->new_item_from_market(from, saleitem->code, saleitem->dna, saleitem->level, saleitem->exp);
        items.erase(saleitem);
        int tax_rate = kv_market_tax_rate;
        if (saleitem->player == self) {
            tax_rate = 0;
        }

        asset price = saleitem->price;
        assert_true(quantity.amount == price.amount, "item price does not match");
        auto dt = time_util::now_shifted();

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
        asset tax(0, eosio::symbol("EOS", 4));
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
        action(permission_level{ self, "active"_n },
               "eosio.token"_n, "transfer"_n,
               std::make_tuple(self, saleitem->player, price, message)
        ).send();

        return tax;
    }

    void sellmat(name from, uint64_t matid, asset price) {
        require_auth(from);
        system_controller.check_blacklist(from);
        require_sell_cooltime(from);

        material_table materials(self, self.value);
        auto imat = materials.find(from.value);
        assert_true(imat != materials.cend(), "no materials");
        auto &rows = imat->rows;
        auto &mat = imat->get_material(matid);

        assert_true(mat.saleid == 0, "already on sale");

        rmaterial_table mat_rules(self, self.value);
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
        assert_true(knights.size() > 0, "there is no knight");

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
        set_sell_factor(from);
    }

    asset buymat(name from, const transfer_action &ad, material_control_actions *ctrl, int max_grade) {
        require_auth(from);
        system_controller.checksum_gateway(from, ad.block, ad.checksum);

        uint64_t saleid = atoll(ad.param.c_str());
        const asset &quantity = ad.quantity;

        auto &players = player_controller.get_players();
        auto player = players.find(from.value);
        assert_true(players.cend() != player, "can not found player info");

        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() > 0, "there is no knight");

        material_table mats(self, self.value);
        auto imat = mats.find(from.value);
        auto current_inventory_size = 0;
        if (imat != mats.cend()) {
            current_inventory_size = imat->rows.size();
        }

        // inventory check
        int max_inventory_size = ctrl->get_max_inventory_size(player->mat_ivn_up);
        assert_true(current_inventory_size < max_inventory_size, "full inventory");

        auto salemat = materials.find(saleid);
        assert_true(salemat != materials.end(), "could not find mat");
        assert_true(salemat->player != from, "it's your mat");
        if (ad.seller.value > 0) {
            //assert_true(salemat->player == ad.seller, "seller not matching");
        }

        // grade check
        assert_true(max_grade > ig_none, "can not buy item this mode");
        if (max_grade != ig_count) {
            rmaterial_table rule_table(self, self.value);
            auto rule = rule_table.find(salemat->code);
            assert_true(rule != rule_table.cend(), "could not find material rule");
            assert_true(rule->grade <= max_grade, "can not buy this item");
        }

        // remove from normal mode
        material_controller.remove_salematerial(salemat->player, salemat->cid);
        ctrl->new_material_from_market(from, salemat->code);
        materials.erase(salemat);

        int tax_rate = kv_market_tax_rate;
        if (salemat->player == self) {
            tax_rate = 0;
        }

        asset price = salemat->price;
        assert_true(quantity.amount == price.amount, "material price does not match");
        
        auto dt = time_util::now_shifted();

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
        asset tax(0, eosio::symbol("EOS", 4));
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

        action(permission_level{ self, "active"_n },
               "eosio.token"_n, "transfer"_n,
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