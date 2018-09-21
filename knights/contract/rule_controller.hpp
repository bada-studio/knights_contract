#pragma once

template <class DataType, class TableType>
class rule_controller {
protected:
    TableType table;
    account_name self;
    name rule_name;

public:
    rule_controller(account_name _self, table_name _rule_name)
            : table(_self, _self)
            , self(_self) {
        rule_name.value = _rule_name;
    }

    void create_rules(const std::vector<DataType> &data, bool truncate) {
        require_auth(self);

        if (truncate) {
            truncate_rules();
        }

        for (auto iter = data.begin(); iter != data.end(); ++iter) {
            table.emplace(self, [&](auto &rule) {
                rule = *iter;
            });
        }

        rversion_table table(self, self);
        auto iter = table.find(rule_name);
        if (iter != table.cend()) {
            table.modify(iter, self, [&](auto &target) {
                target.version++;
            });
        } else {
            table.emplace(self, [&](auto &target) {
                target.rule = rule_name;
                target.version = 1;
            });
        }
    }

    void truncate_rules() {
        require_auth(self);

        auto iter = table.begin();
        while (iter != table.cend()) {
            iter = table.erase(iter);
        }
    }

    TableType& get_table() {
        return table;
    }
};


template <class DataType, class TableType>
class rule_controller2 {
protected:
    TableType table;
    account_name self;
    name rule_name;

    name to_name(account_name target) {
        name res;
        res.value = target;
        return res;
    }

public:
    rule_controller2(account_name _self, table_name _rule_name)
            : table(_self, _self)
            , self(_self) {
        rule_name.value = _rule_name;
    }

    void create_rules(const std::vector<DataType> &rows, bool truncate) {
        require_auth(self);

        if (truncate) {
            truncate_rules();
        }

        table.emplace(self, [&](auto &rule) {
            rule.owner = to_name(self);
            rule.rows = rows;
        });

        rversion_table table(self, self);
        auto iter = table.find(rule_name);
        if (iter != table.cend()) {
            table.modify(iter, self, [&](auto &target) {
                target.version++;
            });
        } else {
            table.emplace(self, [&](auto &target) {
                target.rule = rule_name;
                target.version = 1;
            });
        }
    }

    void truncate_rules() {
        require_auth(self);

        auto iter = table.begin();
        while (iter != table.cend()) {
            iter = table.erase(iter);
        }
    }

    TableType& get_table() {
        return table;
    }
};
