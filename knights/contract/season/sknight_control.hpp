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

    virtual int on_set_enemy_hp(int hp) {
        season_table table(self, self);
        auto season = --table.cend();
        return hp / season->info.speed;
    }

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
};