#pragma once

class control_base {
protected:
    void assert_true(bool test, const char* cstr) {
        eosio::check(test, cstr);
    }

    name to_name(uint64_t target) {
        name res;
        res.value = target;
        return res;
    }

    name get_code_name(eosio::symbol symbol) {
        if (eosio::symbol("EOS", 4) == symbol) {
            return ("eosio.token"_n);
        }

        /*
        switch (symbol) {
            case eosio::symbol("EOS", 4): return "eosio.token"_n;
            case eosio::symbol("BADA", 4): return "thebadatoken"_n;
            case eosio::symbol("TRYBE", 4): return "trybenetwork"_n;
            case eosio::symbol("MEETONE", 4): return "eosiomeetone"_n;
        }

        return 0;
        */
        return ("eosio.token"_n);
    }

    void validate_price(asset price, int grade) {
        assert_true(price.symbol == eosio::symbol("EOS", 4), "only EOS token allowed");
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

    int get_field_material_grade(int code) {
        if (code > 200) {
            return ig_none;
        }

        int index = code % 20;
        if (index <= 4) {
            return ig_normal;
        }
        
        if (index <= 7) {
            return ig_rare;
        }
        
        if (index <= 9) {
            return ig_unique;
        }

        if (index <= 10) {
            return ig_legendary;
        }
        
        if (index <= 11) {
            return ig_ancient;
        }

        return ig_none;
    }
};

class drop_control_base : public control_base {
protected:
    int get_bottie(int grade, random_val &rval) {
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