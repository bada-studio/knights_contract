struct [[eosio::table]] rversion {
    name rule;
    uint16_t version = 0;

    rversion() {
    }

    uint64_t primary_key() const {
        return rule.value;
    }

    EOSLIB_SERIALIZE(
            rversion,
            (rule)
            (version)
    )
};

typedef eosio::multi_index< "rversion"_n, rversion> rversion_table;
