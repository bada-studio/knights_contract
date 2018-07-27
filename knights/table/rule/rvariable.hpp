// Variables are included in code to reduce cpu time. 
// The data in 'rvariable' table is used only by the client. 
// Note the consistency of the data in both.
const int kv_luck_base = 777;
const int kv_defense_base = 1000;
const int kv_enemy_attack = 25;
const int kv_enemy_hp = 200;
const int kv_material_inventory_size = 28;
const int kv_item_inventory_size = 12;
const int kv_bonus_size_for_inventory_up = 4;
const int kv_max_material_inventory_up = 4;
const int kv_max_item_inventory_up = 4;
const int kv_market_tax_rate = 3;
const int kv_pet_gacha_low_price = 100;
const int kv_pet_gacha_high_price = 1000;
const int kv_pet_max_up = 7;
const int kv_max_knight_level = 16;
const int kv_kill_powder_rate = 50;
const int kv_min_market_price =      100;
const int kv_max_market_price = 100'0000;
const int kv_init_powder = 100;
const int kv_min_rebirth = 120;
const int kv_floor_bonus_1000 = 20;
const int kv_max_sales_log_size = 5;
const int kv_available_sale_per_knight = 1;
const int kv_airgrab_level1 = 1;
const int kv_airgrab_level2 = 4;
const int kv_airgrab_level3 = 7;
const int kv_airgrab_amount1 =  4000'0000;
const int kv_airgrab_amount2 =  6000'0000;
const int kv_airgrab_amount3 = 10000'0000;

//@abi table rvariable i64
struct rvariable {
    uint64_t key;
    uint32_t value;

    rvariable() {
    }

    uint64_t primary_key() const {
        return key;
    }

    EOSLIB_SERIALIZE(
            rvariable,
            (key)
            (value)
    )
};

typedef eosio::multi_index< N(rvariable), rvariable> rvariable_table;
