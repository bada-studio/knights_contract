#pragma once

class control_base {
protected:
    void assert_true(bool test, const char* cstr) {
        eosio_assert(test ? 1 : 0, cstr);
    }

    name to_name(account_name target) {
        name res;
        res.value = target;
        return res;
    }
};