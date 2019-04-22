// 10 bytes
struct matrow {
    uint32_t id = 0;
    uint16_t code = 0;
    uint32_t saleid = 0;
};

//@abi table material i64
//@abi table smaterial i64
struct material {
    name owner;
    uint32_t last_id;
    std::vector<matrow> rows;

    uint64_t primary_key() const {
        return owner;
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
        
        eosio_assert(0, "can not found material");
        return rows[0]; // never happen
    }

    EOSLIB_SERIALIZE(
            material,
            (owner)
            (last_id)
            (rows)
    )
};

typedef eosio::multi_index< N(material), material> material_table;
typedef eosio::multi_index< N(smaterial), material> smaterial_table;
