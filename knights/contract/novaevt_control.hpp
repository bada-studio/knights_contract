#pragma once

class novaevt_control : public control_base {
private:
    account_name self;
    system_control &system_controller;

public:
    /// @brief
    /// Constructor
    novaevt_control(account_name _self,
                  system_control &_system_controller)
            : self(_self)
            , system_controller(_system_controller) {
    }

    // actions
    //-------------------------------------------------------------------------
    void getnova(name from, const std::string &memo) {
        require_auth(N(novapromote1));

        auto player = system_controller.get_player(from);
        if (system_controller.is_empty_player(player)) {
            system_controller.new_player(from);
            player = system_controller.get_player(from);
        }

        novaevt_table table(self, self);
        assert_true(table.cbegin() != table.cend(), "#EOSNIGHTSERROR# No nova data yet");
        
        auto iter = --table.cend();
        assert_true(iter->remain >= iter->amount, "#EOSNIGHTSERROR# We have run out of magic water");

        table.modify(iter, self, [&](auto &target) {
            target.remain -= target.amount;
        });

        system_controller.increase_powder(player, iter  ->amount);
    }

    void addnova(uint64_t id, uint32_t total, uint32_t remain, uint32_t amount) {
        require_auth(self);
        
        novaevt_table table(self, self);
        auto iter = table.find(id);
        if (iter == table.cend()) {
            if (table.cbegin() != table.cend()) {
                auto iter = --table.cend();
                assert_true(iter->remain < iter->amount, "there is remain nova");
            }

            table.emplace(self, [&](auto &target) {
                target.id = id;
                target.total = total;
                target.remain = remain;
                target.amount = amount;
            });
        } else {
            table.modify(iter, self, [&](auto &target) {
                target.total = total;
                target.remain = remain;
                target.amount = amount;
            });
        }
    }
};
