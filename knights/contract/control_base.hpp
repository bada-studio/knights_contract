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

class drop_control_base : public control_base {
protected:
    int get_bottie(const player& player, int grade, random_val &rval) {
        int start = 0;
        int length = 0;

        double ndr[10] = {0.0, };
        switch (grade) {
            case ig_normal:
                start = 0;
                length = 4;
                break;
            case ig_rare:
                start = 4;
                length = 3;
                break;
            case ig_unique:
                start = 7;
                length = 2;
                break;
            case ig_legendary:
                start = 9;
                length = 1;
                break;
            case ig_ancient:
                start = 10;
                length = 1;
                break;
        }

        // copy drop rate
        double sum = 0;
        for (int index = 0; index < length; index++) {
            double value = drop_rates[start + index];
            sum += value;
            ndr[index] = value;
        }

        // normalize drop rate
        for (int index = 0; index < length; index++) {
            ndr[index] /= sum;
        }

        int best = 0;
        int drscale = 1000000000;
        int rand_value = rval.range(drscale);

        for (int index = length-1; index >= 1; --index) {
            if (rand_value < int(ndr[index] * drscale)) {
                best = index;
                break;
            }
        }

        int type = rval.range(5) + 1;
        int code = (type - 1) * 20 + (best + start + 1);
        return code;
    }    
};