#pragma once

/*
 * season mode knight controller
 */
class sknight_control : public knight_control_base<
    sknight_table, 
    sknight_table::const_iterator, 
    splayer,
    splayer_table::const_iterator,
    splayer_control,
    smaterial_control,
    sitem_control,
    spet_control> {

public:
    // constructor
    //-------------------------------------------------------------------------
    sknight_control(account_name _self,
                    system_control &_system_controller,
                    splayer_control &_player_controller,
                    smaterial_control &_material_controller,
                    sitem_control &_item_controller,
                    spet_control &_pet_controller)
            : knight_control_base(
                _self,
                _system_controller,
                _player_controller,
                _material_controller,
                _item_controller,
                _pet_controller) {
        use_gdr = false;
    }
    
    // speed boost
    virtual int filter_enemy_hp(int hp) {
        season_table table(self, self);
        auto season = --table.cend();
        return hp / season->info.speed;
    }

    // set rebirth factor to splayer instead of playerv2
    virtual void do_check_rebirth_factor(splayer_table::const_iterator player, 
                                         const std::vector<knightrow> &rows,
                                         playerv2 &variable) {
        auto prev_factor = player->rebrith_factor;
        auto new_factor = calc_rebirth_factor(player, rows, prev_factor);

        auto &players = player_controller.get_players();
        players.modify(player, self, [&](auto &target) {
            target.rebrith_factor = new_factor;
        });
    }

    virtual void on_rebirth_done(name from, int old_max_floor, int floor, const std::vector<knightrow> &knts) {
        if (floor <= old_max_floor) {
            return;
        }

        season_table table(self, self);
        if (table.cbegin() == table.cend()) {
            return;
        }

        auto season = --table.cend();

        // check it's new record
        auto max_count = season->info.max_record_count;
        auto &records = season->state.records;
        auto current_size = records.size();
        if (current_size >= max_count) {
            if (records[current_size-1].floor >= floor) {
                return;
            }
        }

        // find place to save
        int remove_pos = -1;
        int insert_pos = current_size;

        for (int k = 0; k < current_size; k++) {
            auto &record = records[k];
            if (record.floor < floor) {
                if (k < insert_pos) {
                    insert_pos = k;
                }
            }

            // same owner
            if (record.owner == from) {
                remove_pos = k;
            }
        }

        if (remove_pos > 0 && remove_pos < insert_pos) {
            insert_pos--;
        }

        auto adopted = pet_controller.get_all_knight_pet(from);
        auto equip = item_controller.get_all_knight_item(from);

        // update point
        table.modify(season, self, [&](auto &target) {
            auto &records = target.state.records;
            if (remove_pos >= 0) {
                records.erase(records.cbegin() + remove_pos);
            }

            seasonrecord record;
            record.owner = from;
            record.floor = floor;
            record.knights = knts;
            record.equip = equip;
            record.adopted = adopted;
            record.paid = false;

            records.insert(records.cbegin() + insert_pos, record);

            // trim
            if (records.size() > max_count) {
                records.erase(--records.cend());
            }
        });
    }
};