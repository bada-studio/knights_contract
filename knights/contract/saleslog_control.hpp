#pragma once

class saleslog_control : public control_base {
private:
    account_name self;
    revenue_table revenues;
    
public:
    saleslog_control(account_name _self) 
        : self(_self)
        , revenues(_self, _self) {
    }

    void add_saleslog(selllog log, name from) {
        auto iter = revenues.find(from);
        if (iter == revenues.cend()) {
            revenues.emplace(self, [&](auto& target) {
                target.owner = from;
                target.selling += log.price;
                target.selling_count = 1;
                target.selllogs.push_back(log);
            });
        } else {
            revenues.modify(iter, self, [&](auto& target) {
                if (target.selllogs.size() >= kv_max_sales_log_size) {
                    target.selllogs.erase(target.selllogs.begin());
                }
                target.selling += log.price;
                target.selling_count++;
                target.selllogs.push_back(log);
            });
        }
    }

    void add_buylog(buylog log, name from) {
        auto iter = revenues.find(from);
        if (iter == revenues.cend()) {
            revenues.emplace(self, [&](auto& target) {
                target.owner = from;
                if (log.type == ct_material || log.type == ct_item) {
                    target.buying += log.price;
                    target.buying_count = 1;
                } else {
                    target.spending += log.price;
                    target.spending_count = 1;
                }
                target.buylogs.push_back(log);
            });
        } else {
            revenues.modify(iter, self, [&](auto& target) {
                if (target.buylogs.size() >= kv_max_sales_log_size) {
                    target.buylogs.erase(target.buylogs.begin());
                }
                if (log.type == ct_material || log.type == ct_item) {
                    target.buying += log.price;
                    target.buying_count++;
                } else {
                    target.spending += log.price;
                    target.spending_count++;
                }
                target.buylogs.push_back(log);
            });
        }
    }
};