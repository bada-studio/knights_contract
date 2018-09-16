//@ warning for the performance issue, drop rates are hard coded here,
// be careful for the data sync with material rule
const int drop_rates_length = 11;
const double drop_rates[drop_rates_length] = {
    0.5255616988, // 0
    0.2627808494, // 1
    0.1313904247, // 2
    0.06569521235, // 3
    0.008211901543, // 4
    0.004105950772, // 5
    0.002052975386, // 6
    0.0001283109616, // 7
    0.00006415548081, // 8
    0.000008019435101, // 9
    0.0000005012146938, // 10
};

enum material_type {
    mt_none = 0,
    mt_nature, // 1
    mt_iron, // 2
    mt_bone, // 3
    mt_skin, // 4
    mt_mineral, // 5
};

//@abi table rmaterial i64
struct rmaterial {
    uint64_t code = 0;
    uint8_t type = 0;
    uint8_t grade = 0;
    uint32_t relative_drop_rate = 0;
    uint16_t powder = 0;

    rmaterial() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rmaterial,
            (code)
            (type)
            (grade)
            (relative_drop_rate)
            (powder)
    )
};

typedef eosio::multi_index<N(rmaterial), rmaterial> rmaterial_table;
