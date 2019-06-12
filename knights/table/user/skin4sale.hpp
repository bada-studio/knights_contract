struct skin4salerow {
    uint64_t mid = 0;
    uint32_t cid = 0;
    uint16_t code = 0;
    name seller;
    asset price;
};

struct [[eosio::table]] skin4sale {
    uint64_t code = 0;
    uint64_t last_mid = 0;
    std::vector<skin4salerow> rows;

    skin4sale() {
    }

    uint64_t primary_key() const {
        return code;
    }

    int get_skin_by_mid(uint64_t mid) const {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].mid == mid) { 
                return index;
            }
        }
        return -1;
    }

    int get_skin_by_cid(uint64_t cid) const {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].cid == cid) { 
                return index;
            }
        }
        return -1;
    }

    EOSLIB_SERIALIZE(
            skin4sale,
            (code)
            (last_mid)
            (rows)
    )
};

typedef eosio::multi_index< "skin4sale"_n, skin4sale > skin4sale_table;
