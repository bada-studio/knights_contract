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
const int kv_max_material_inventory_up = 5;
const int kv_max_item_inventory_up = 5;
const int kv_market_tax_rate = 5;
const int kv_pet_gacha_low_price = 100;
const int kv_pet_gacha_high_price = 1000;
const int kv_required_floor_for_material = 0x521;
const int kv_min_market_price_scaler = 0x2525;
const int kv_skill_reset_price = 1;
const int kv_alchemist_count = 0x804; // max, min (8/4)
const int kv_alchemist_rate = 0x264B50;  // legendary, unique, rare (38/75/80)
const int kv_max_knight_level = 18;
const int kv_kill_powder_rate = 50;
const int kv_min_market_price = 10;
const int kv_max_market_price = 10000000;
const int kv_init_powder = 100;
const int kv_min_rebirth = 120;
const int kv_floor_bonus_1000 = 20;
const int kv_max_sales_log_size = 5;
const int kv_pet_exp_duration = 0x85432;
const int kv_dungeon_free_ticket = 0x1808;
const int kv_dungeon_max_ticket = 8;
const int kv_pet_attach_bonus = 120;
const int kv_referral_bonus = 1000;
const int kv_referral_max = 100;
const int kv_bonus_sell1_floor =  700;
const int kv_bonus_sell2_floor = 1500;
const int kv_pet_max_up = 0x35789;
const int kv_pet_exp_require_level = 0xa731;
const int kv_comment_cost = 5000;
const int kv_season_mat_factor = 1032;

const int32_t checksum_mask = 2042423;

enum variable_type {
    vt_none = 0,
    vt_luck_base,
    vt_defense_base,
    vt_enemy_attack,
    vt_enemy_hp,
    vt_material_inventory_size,
    vt_item_inventory_size,
    vt_bonus_size_for_inventory_up,
    vt_max_material_inventory_up,
    vt_max_item_inventory_up,
    vt_market_tax_rate,
    vt_pet_gacha_low_price,
    vt_pet_gacha_high_price,
    vt_required_floor_for_material,
    vt_min_market_price_scaler,
    vt_skill_reset_price,
    vt_pet_legendary_max_up,
    vt_pet_ancient_max_up,
    vt_max_knight_level,
    vt_kill_powder_rate,
    vt_min_market_price,
    vt_max_market_price,
    vt_init_powder,
    vt_min_rebirth,
    vt_floor_bonus_1000,
    vt_max_sales_log_size,
    vt_available_sale_per_knight,
    vt_required_floor_for_unique,
    vt_required_floor_for_legendary,
    vt_required_floor_for_ancient,
};

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
