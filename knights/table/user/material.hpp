// 10 bytes
struct matrow {
    uint32_t id = 0;
    uint16_t code = 0;
    uint32_t saleid = 0;
};

// todo check
// table material i64
// table smaterial i64
struct [[eosio::table]] material {
    name owner;
    uint32_t last_id;
    std::vector<matrow> rows;

    uint64_t primary_key() const {
        return owner.value;
    }

    const matrow& get_material(int id) const {
        // binary search
        int left = 0;
        int right = rows.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (rows[mid].id < id) {
                left = mid + 1;
            } else if (id < rows[mid].id) {
                right = mid - 1;
            } else {
                return rows[mid];
            }
        }
        
        eosio::check(false, "can not found material");
        return rows[0]; // never happen
    }

    EOSLIB_SERIALIZE(
            material,
            (owner)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< "material"_n, material> material_table;
typedef eosio::multi_index< "smaterial"_n, material> smaterial_table;
