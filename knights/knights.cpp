#include <vector>
#include <string>
#include <eosio/eosio.hpp>
#include <eosio/symbol.hpp>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>

#include <cmath>

#define MAINTENANCE 0
#define USE_DEFERRED 1

using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::name;

#include "table/rule/rivnprice.hpp"
#include "table/rule/rkntlv.hpp"
#include "table/rule/rkntprice.hpp"
#include "table/rule/rknt.hpp"
#include "table/rule/rstage.hpp"
#include "table/rule/rvariable.hpp"
#include "table/rule/ritem.hpp"
#include "table/rule/ritemlv.hpp"
#include "table/rule/ritemset.hpp"
#include "table/rule/rmaterial.hpp"
#include "table/rule/rpet.hpp"
#include "table/rule/rpetlv.hpp"
#include "table/rule/rpetexp.hpp"
#include "table/rule/rmpgoods.hpp"
#include "table/rule/rkntskill.hpp"
#include "table/rule/rdungeon.hpp"
#include "table/rule/rdgticket.hpp"
#include "table/rule/rmob.hpp"
#include "table/rule/rmobskill.hpp"
#include "table/user/player.hpp"
#include "table/user/playerv.hpp"
#include "table/user/comment.hpp"
#include "table/user/knight.hpp"
#include "table/user/material.hpp"
#include "table/user/mat4sale.hpp"
#include "table/user/item.hpp"
#include "table/user/item4sale.hpp"
#include "table/user/pet.hpp"
#include "table/user/petexp.hpp"
#include "table/user/revenue.hpp"
#include "table/user/kntskill.hpp"
#include "table/user/dungeon.hpp"
#include "table/user/skin.hpp"
#include "table/user/skin4sale.hpp"
#include "table/user/skininfo.hpp"
#include "table/user/medal.hpp"
#include "table/outchain/knight_stats.hpp"
#include "table/outchain/transfer_action.hpp"
#include "table/outchain/random_val.hpp"
#include "table/outchain/dgorder.hpp"
#include "table/admin/adminstate.hpp"
#include "table/admin/revenuedt.hpp"
#include "table/admin/stockholder.hpp"
#include "table/admin/dividendlog.hpp"
#include "table/admin/expenseslog.hpp"
#include "table/admin/rversion.hpp"
#include "table/admin/marketpid.hpp"
#include "table/admin/gift.hpp"
#include "table/admin/cquest.hpp"
//#include "table/admin/dquest.hpp"
#include "table/admin/season.hpp"
#include "table/admin/globalvar.hpp"
#include "table/admin/itemevt.hpp"
#include "util/time_util.hpp"
#include "contract/control_base.hpp"
#include "contract/admin_control.hpp"
#include "contract/rule_controller.hpp"
#include "contract/saleslog_control.hpp"
#include "contract/player_control.hpp"
#include "contract/season/splayer_control.hpp"
#include "contract/system_control.hpp"
#include "contract/material_control.hpp"
#include "contract/season/smaterial_control.hpp"
#include "contract/item_control.hpp"
#include "contract/season/sitem_control.hpp"
#include "contract/pet_control.hpp"
#include "contract/season/spet_control.hpp"
#include "contract/knight_control.hpp"
#include "contract/season/sknight_control.hpp"
#include "contract/market_control.hpp"
#include "contract/cquest_control.hpp"
//#include "contract/dquest_control.hpp"
#include "contract/season/season_control.hpp"
#include "contract/system_control.cpp"
#include "contract/dungeon_control.hpp"
//#include "contract/itemevt_control.hpp"
#include "contract/skin_control.hpp"
//#include "table/admin/novaevt.hpp"
//#include "contract/novaevt_control.hpp"

class [[eosio::contract]] knights : public eosio::contract, public control_base {
private:
    name self;

    // controls
    system_control system_controller;
    player_control player_controller;
    splayer_control splayer_controller;
    material_control material_controller;
    pet_control pet_controller;
    knight_control knight_controller;
    sknight_control sknight_controller;
    item_control item_controller;
    sitem_control sitem_controller;
    market_control market_controller;
    admin_control admin_controller;
    saleslog_control saleslog_controller;
    cquest_control cquest_controller;
//    dquest_control dquest_controller;
    season_control season_controller;
    smaterial_control smaterial_controller;
    spet_control spet_controller;
    dungeon_control dungeon_controller;
    //itemevt_control itemevt_controller; 
    skin_control skin_controller;

    const char* ta_knt = "knt";
    const char* ta_mw = "mw";
    const char* ta_dmw = "dmw";
    const char* ta_item = "item";
    const char* ta_item_season = "item-season";
    const char* ta_mat = "mat";
    const char* ta_mat_season = "mat-season";
    const char* ta_skin = "skin";
    const char* ta_ivn = "ivn";
    const char* tp_item = "item";
    const char* tp_mat = "mat";
    const char* ta_eosknights = "eosknights";
    const char* tp_mat_sale = "material-sale";

public:
    knights(name s, name code, eosio::datastream<const char*> ds)
    : contract(s, code, ds)
    , self(_self)
    , admin_controller(s)
    , saleslog_controller(s)
    , player_controller(s, saleslog_controller)
    , splayer_controller(s, saleslog_controller)
    , system_controller(s, player_controller, saleslog_controller, admin_controller)
    , material_controller(s, system_controller, player_controller)
    , smaterial_controller(s, system_controller, splayer_controller)
    , item_controller(s, system_controller, player_controller, material_controller)
    , sitem_controller(s, system_controller, splayer_controller, smaterial_controller)
    , pet_controller(s, system_controller, player_controller, material_controller)
    , spet_controller(s, system_controller, splayer_controller, smaterial_controller)
    , knight_controller(s, system_controller, player_controller, material_controller, item_controller, pet_controller, saleslog_controller)
    , sknight_controller(s, system_controller, splayer_controller, smaterial_controller, sitem_controller, spet_controller)
    , market_controller(s, system_controller, player_controller, material_controller, item_controller, saleslog_controller, knight_controller)
    , cquest_controller(s, item_controller, system_controller, admin_controller)
//    , dquest_controller(s, item_controller, system_controller, admin_controller)
    , season_controller(s, system_controller, admin_controller, sknight_controller, sitem_controller)
    , dungeon_controller(s, system_controller, player_controller, material_controller, knight_controller/*, dquest_controller*/)
    //, itemevt_controller(s, system_controller, player_controller, item_controller)
    , skin_controller(s, system_controller, saleslog_controller) {
    }

    // player related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void signup(name from) {
        require_auth(from);
        system_controller.signup(from);
        knight_controller.new_free_knight(from);
    }

    [[eosio::action]]
    void signupbt(name from) {
        require_auth("bastetbastet"_n);
        system_controller.signup(from);
        knight_controller.new_free_knight(from);
    }

    [[eosio::action]]
    void referral(name from, name to) {
        system_controller.referral(from, to);
    }

    [[eosio::action]]
    void getgift(name from, int16_t no) {
        system_controller.getgift(from, no);
    }

    [[eosio::action]]
    void addgift(uint16_t no, uint8_t type, uint16_t amount, uint32_t to) {
        system_controller.addgift(no, type, amount, to);
    }

    [[eosio::action]]
    void addcomment(name from, const std::string& message, const std::string& link) {
        system_controller.addcomment(from, message, link);
    }

    [[eosio::action]]
    void reportofs(name from, name to) {
        system_controller.reportofs(from, to);
    }

    [[eosio::action]]
    void addblackcmt(name to) {
        system_controller.addblackcmt(to);
    }

    // season related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void addseason(bool add, const seasoninfo &info) {
        season_controller.addseason(add, info);
    }

    [[eosio::action]]
    void joinseason(name from) {
        season_controller.joinseason(from);
    }

    [[eosio::action]]
    void seasonreward(name from, uint32_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        season_controller.seasonreward(from, id);
    }

    [[eosio::action]]
    void submitsq(name from, int32_t season, int32_t id, uint32_t block, uint32_t checksum) {
        require_season(season);
        auto knt = season_controller.submitsq(from, id);
        if (knt > 0) {
            sknight_controller.refresh_stat(from, knt);
        }
    }

    seasoninfo require_season(uint32_t season) {
        season_table table(self, self.value);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto iter = --table.cend();
        assert_true(iter->id == season, "invalid season");
        assert_true(iter->info.is_in(time_util::now()), "not in season period");
        return iter->info;
    }

    seasoninfo require_season_open() {
        season_table table(self, self.value);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto iter = --table.cend();
        auto now = time_util::now();
        assert_true(iter->info.is_in(now), "not in season period");
        return iter->info;
    }

    // cquest related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void addcquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        cquest_controller.addcquest(id, sponsor, start, duration);
    }

    // void removecquest(uint32_t id, bool force) {
    //    // it only available there is no user's record
    //    cquest_controller.removecquest(id, force);
    //}

    [[eosio::action]]
    void updatesubq(uint32_t id, const std::vector<cquestdetail>& details) {
        cquest_controller.updatesubq(id, details);
    }

    [[eosio::action]]
    void submitcquest(name from, uint32_t cquest_id, uint8_t no, uint32_t item_id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        cquest_controller.submitcquest(from, cquest_id, no, item_id);
    }

    [[eosio::action]]
    void divcquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        cquest_controller.divcquest(id, no, from, count);
    }

    // dquest related actions
    //-------------------------------------------------------------------------
    /*
    [[eosio::action]]
    void adddquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        dquest_controller.adddquest(id, sponsor, start, duration);
    }

    //void removedquest(uint32_t id, bool force) {
    //    // it only available there is no user's record
    //    dquest_controller.removedquest(id, force);
    //}

    [[eosio::action]]
    void updatedsubq(uint32_t id, const std::vector<dquestdetail>& details) {
        dquest_controller.updatedsubq(id, details);
    }

    [[eosio::action]]
    void divdquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        dquest_controller.divdquest(id, no, from, count);
    }

    */

    // knight related actions
    //-------------------------------------------------------------------------
    knight_control_actions* get_knight_ctl(uint32_t season) {
        if (season == 0) {
            return &knight_controller;
        }

        require_season(season);
        return &sknight_controller;
    }

    [[eosio::action]]
    void lvupknight3(name from, uint32_t season, uint8_t type) {
        get_knight_ctl(season)->lvupknight(from, type);
    }

    [[eosio::action]]
    void rebirth3(name from, uint32_t season, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_knight_ctl(season)->rebirth(from, season, checksum, true);
    }

    [[eosio::action]]
    void rebirth3i(name from, uint32_t season, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_knight_ctl(season)->rebirth(from, season, checksum, false);
    }

    [[eosio::action]]
    void equip3(name from, uint32_t season, uint8_t knight, uint32_t id) {
        get_knight_ctl(season)->equip(from, knight, id);
    }

    [[eosio::action]]
    void detach3(name from, uint32_t season, uint32_t id) {
        get_knight_ctl(season)->detach(from, id);
    }

    [[eosio::action]]
    void setkntstage(name from, uint8_t stage) {
        knight_controller.setkntstage(from, stage);
    }

    [[eosio::action]]
    void skillup(name from, uint8_t knt, uint16_t id) {
        knight_controller.skillup(from, knt, id);
    }

    [[eosio::action]]
    void skillreset(name from, uint8_t knt) {
        knight_controller.skillreset(from, knt);
    }

    // material related actions
    //-------------------------------------------------------------------------
    material_control_actions* get_material_ctl(uint32_t season) {
        if (season == 0) {
            return &material_controller;
        }

        require_season(season);
        return &smaterial_controller;
    }

    [[eosio::action]]
    void removemat3(name from, uint32_t season, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_material_ctl(season)->remove(from, mat_ids);
    }

    [[eosio::action]]
    void alchemist(name from, uint32_t grade, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        material_controller.alchemist(from, grade, mat_ids, checksum, true);
    }

    [[eosio::action]]
    void alchemisti(name from, uint32_t grade, const std::vector<uint32_t>& mat_ids, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        material_controller.alchemist(from, grade, mat_ids, checksum, false);
    }

    // item related actions
    //-------------------------------------------------------------------------
    item_control_actions* get_item_ctl(uint32_t season) {
        if (season == 0) {
            return &item_controller;
        }

        require_season(season);
        return &sitem_controller;
    }

    [[eosio::action]]
    void craft3(name from, uint32_t season, uint16_t code, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_item_ctl(season)->craft(from, season, code, mat_ids, checksum, true);
    }

    [[eosio::action]]
    void craft3i(name from, uint32_t season, uint16_t code, const std::vector<uint32_t>& mat_ids, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_item_ctl(season)->craft(from, season, code, mat_ids, checksum, false);
    }

    [[eosio::action]]
    void itemlvup3(name from, uint32_t season, uint32_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        int8_t knight = get_item_ctl(season)->itemlvup(from, season, id, checksum, true);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    [[eosio::action]]
    void itemlvup3i(name from, uint32_t season, uint32_t id, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        int8_t knight = get_item_ctl(season)->itemlvup(from, season, id, checksum, false);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    [[eosio::action]]
    void removeitem3(name from, uint32_t season, const std::vector<uint32_t>& item_ids) {
        get_item_ctl(season)->remove(from, item_ids);
    }

    [[eosio::action]]
    void itemmerge3(name from, uint32_t season, uint32_t id, const std::vector<uint32_t> &ingredient) {
        get_item_ctl(season)->itemmerge(from, id, ingredient);
    }

    // pet related actions
    //-------------------------------------------------------------------------
    pet_control_actions* get_pet_ctl(uint32_t season) {
        if (season == 0) {
            return &pet_controller;
        }

        auto info = require_season(season);
        assert_true(info.opt_no_pet == false, "can not use pet this mode");
        return &spet_controller;
    }

    [[eosio::action]]
    void petgacha3(name from, uint32_t season, uint16_t type, uint8_t count, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() > 0, "hire knight first!");
        get_pet_ctl(season)->petgacha(from, season, type, count, checksum, true);
    }

    [[eosio::action]]
    void petgacha3i(name from, uint32_t season, uint16_t type, uint8_t count, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_pet_ctl(season)->petgacha(from, season, type, count, checksum, false);
    }

    [[eosio::action]]
    void petlvup3(name from, uint32_t season, uint16_t code) {
        int8_t knight = get_pet_ctl(season)->petlvup(from, code);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    [[eosio::action]]
    void pattach3(name from, uint32_t season, uint16_t code, uint8_t knight) {
        get_pet_ctl(season)->pattach(from, code, knight);
        get_knight_ctl(season)->refresh_stat(from, knight);
    }

    [[eosio::action]]
    void pexpstart2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        auto knight_max_level = knight_controller.get_knight_max_level(from);
        pet_controller.pexpstart(from, code, knight_max_level);
    }

    [[eosio::action]]
    void pexpreturn2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        pet_controller.pexpreturn(from, code, checksum, true);
    }

    [[eosio::action]]
    void pexpreturn2i(name from, uint16_t code, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        pet_controller.pexpreturn(from, code, checksum, false);
    }

    // market related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void sellitem2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.sellitem(from, id, price);
    }

    [[eosio::action]]
    void ccsellitem2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellitem(from, id);
    }

    [[eosio::action]]
    void sellmat2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.sellmat(from, id, price);
    }

    [[eosio::action]]
    void ccsellmat2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellmat(from, id);
    }

    // dungeon related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void dgtcraft(name from, uint16_t code, const std::vector<uint32_t> &mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgtcraft(from, code, mat_ids);
    }

    [[eosio::action]]
    void dgfreetk2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgfreetk(from, code);
    }

    [[eosio::action]]
    void dgenter(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgenter(from, code);
    }

    [[eosio::action]]
    void dgclear(name from, uint16_t code, const std::vector<uint32_t> orders, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgclear(from, code, orders, checksum, true, false);
    }

    [[eosio::action]]
    void dgcleari(name from, uint16_t code, const std::vector<uint32_t> orders, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        dungeon_controller.dgclear(from, code, orders, checksum, false, false);
    }

    [[eosio::action]]
    void dgleave(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgleave(from, code);
    }    

    // skin related actions
    //-------------------------------------------------------------------------
    [[eosio::action]]
    void skissue(uint16_t code, uint32_t count, asset price) {
        skin_controller.skissue(code, count, price);
    }

    [[eosio::action]]
    void sksell(name from, uint32_t cid, asset price) {
        skin_controller.sksell(from, cid, price);
    }

    [[eosio::action]]
    void skcsell(name from, uint32_t cid) {
        skin_controller.skcsell(from, cid);
    }

    [[eosio::action]]
    void skwear(name from, uint32_t knt, uint32_t cid) {
        skin_controller.skwear(from, knt, cid);
    }

    // rule related actions
    //-------------------------------------------------------------------------
    /*
    [[eosio::action]]
    void civnprice(const std::vector<rivnprice> &rules, bool truncate) {
        rule_controller<rivnprice, rivnprice_table> controller(self, "ivnprice"_n);
        controller.create_rules(rules, truncate);
    }
    /*/
    /*
    [[eosio::action]]
    void cknt(const std::vector<rknt> &rules, bool truncate) {
        rule_controller<rknt, rknt_table> controller(self, "knt"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void ckntlv(const std::vector<rkntlv> &rules, bool truncate) {
        rule_controller<rkntlv, rkntlv_table> controller(self, "kntlv"_n);
        controller.create_rules(rules, truncate);
    }
    */
    /*
    [[eosio::action]]
    void ckntprice(const std::vector<rkntprice> &rules, bool truncate) {
        rule_controller<rkntprice, rkntprice_table> controller(self, "kntprice"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void ckntskills(const std::vector<rkntskills> &rules, bool truncate) {
        rule_controller<rkntskills, rkntskills_table> controller(self, "kntskills"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cstage(const std::vector<rstage> &rules, bool truncate) {
        rule_controller<rstage, rstage_table> controller(self, "stage"_n);
        controller.create_rules(rules, truncate);
    }
    */
    [[eosio::action]]
    void cvariable(const std::vector<rvariable> &rules, bool truncate) {
        rule_controller<rvariable, rvariable_table> controller(self, "variable"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void citem(const std::vector<ritem> &rules, bool truncate) {
        rule_controller<ritem, ritem_table> controller(self, "item"_n);
        controller.create_rules(rules, truncate);
    }

    /*
    [[eosio::action]]
    void citemlv(const std::vector<ritemlv> &rules, bool truncate) {
        rule_controller<ritemlv, ritemlv_table> controller(self, "itemlv"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void citemset(const std::vector<ritemset> &rules, bool truncate) {
        rule_controller<ritemset, ritemset_table> controller(self, "itemset"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cmaterial(const std::vector<rmaterial> &rules, bool truncate) {
        rule_controller<rmaterial, rmaterial_table> controller(self, "material"_n);
        controller.create_rules(rules, truncate);
    }
    
    [[eosio::action]]
    void cpet(const std::vector<rpet> &rules, bool truncate) {
        rule_controller<rpet, rpet_table> controller(self, "pet"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cpetlv(const std::vector<rpetlv> &rules, bool truncate) {
        rule_controller<rpetlv, rpetlv_table> controller(self, "petlv"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cpetexp(const std::vector<rpetexp> &rules, bool truncate) {
        rule_controller<rpetexp, rpetexp_table> controller(self, "petexp"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cmpgoods(const std::vector<rmpgoods> &rules, bool truncate) {
        rule_controller<rmpgoods, rmpgoods_table> controller(self, "mpgoods"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cdungeon(const std::vector<rdungeon> &rules, bool truncate) {
        rule_controller<rdungeon, rdungeon_table> controller(self, "dungeon"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cdgticket(const std::vector<rdgticket> &rules, bool truncate) {
        rule_controller<rdgticket, rdgticket_table> controller;(self, "dgticket"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cmobs(const std::vector<rmobs> &rules, bool truncate) {
        rule_controller<rmobs, rmobs_table> controller(self, "mobs"_n);
        controller.create_rules(rules, truncate);
    }

    [[eosio::action]]
    void cmobskills(const std::vector<rmobskills> &rules, bool truncate) {
        rule_controller<rmobskills, rmobskills_table> controller(self, "mobskills"_n);
        controller.create_rules(rules, truncate);
    }
    */
   
    [[eosio::action]]
    void trule(name table, uint16_t size) {
        if (table == "ivnprice"_n) {
            //rule_controller<rivnprice, rivnprice_table> controller(self, "ivnprice"_n);
            //controller.truncate_rules(size);
        } else if (table == "knt"_n) {
            //rule_controller<rknt, rknt_table> controller(self, "knt"_n);
            //controller.truncate_rules(size);
        } else if (table == "kntlv"_n) {
            //rule_controller<rkntlv, rkntlv_table> controller(self, "kntlv"_n);
            //controller.truncate_rules(size);
        } else if (table == "kntprice"_n) {
            //rule_controller<rkntprice, rkntprice_table> controller(self, "kntprice"_n);
            //controller.truncate_rules(size);
        } else if (table == "kntskills"_n) {
            //rule_controller<rkntskills, rkntskills_table> controller(self, "kntskills"_n);
            //controller.truncate_rules(size);
        } else if (table == "stage"_n) {
            //rule_controller<rstage, rstage_table> controller(self, "stage"_n);
            //controller.truncate_rules(size);
        } else if (table == "variable"_n) {
            rule_controller<rvariable, rvariable_table> controller(self, "variable"_n);
            controller.truncate_rules(size);
        } else if (table == "item"_n) {
            rule_controller<ritem, ritem_table> controller(self, "item"_n);
            controller.truncate_rules(size);
        } else if (table == "itemlv"_n) {
            //rule_controller<ritemlv, ritemlv_table> controller(self, "itemlv"_n);
            //controller.truncate_rules(size);
        } else if (table == "itemset"_n) {
            //rule_controller<ritemset, ritemset_table> controller(self, "itemset"_n);
            //controller.truncate_rules(size);
        } else if (table == "material"_n) {
            //rule_controller<rmaterial, rmaterial_table> controller(self, "material"_n);
            //controller.truncate_rules(size);
        } else if (table == "pet"_n) {
            //rule_controller<rpet, rpet_table> controller(self, "pet"_n);
            //controller.truncate_rules(size);
        } else if (table == "petlv"_n) {
            //rule_controller<rpetlv, rpetlv_table> controller(self, "petlv"_n);
            //controller.truncate_rules(size);
        } else if (table == "petexp"_n) {
            //rule_controller<rpetexp, rpetexp_table> controller(self, "petexp"_n);
            //controller.truncate_rules(size);
        } else if (table == "mpgoods"_n) {
            //rule_controller<rmpgoods, rmpgoods_table> controller(self, "mpgoods"_n);
            //controller.truncate_rules(size);
        } else if (table == "dungeon"_n) {
            //rule_controller<rdungeon, rdungeon_table> controller(self, "dungeon"_n);
            //controller.truncate_rules(size);
        } else if (table == "dgticket"_n) {
            //rule_controller<rdgticket, rdgticket_table> controller;(self, "dgticket"_n);
            //controller.truncate_rules(size);
        } else if (table == "mobs"_n) {
            //rule_controller<rmobs, rmobs_table> controller(self, "mobs"_n);
            //controller.truncate_rules(size);
        } else if (table == "mobskills"_n) {
            //rule_controller<rmobskills, rmobskills_table> controller(self, "mobskills"_n);
            //controller.truncate_rules(size);
        } else {
            eosio::check(false, "could not find table");
        }
    }

    // admin related actions
    //-------------------------------------------------------------------------
    // void setpause(uint8_t pid) {
    //    admin_controller.setpause(pid);
    // }

    [[eosio::action]]
    void setcoo(name coo) {
        admin_controller.setcoo(coo);
    }

    [[eosio::action]]
    void regsholder(name holder, uint16_t share) {
        admin_controller.regsholder(holder, share);
    }

    [[eosio::action]]
    void dividend(asset amount) {
        require_auth(self);
        admin_controller.dividend(amount);
    }

    // etc actions
    //-------------------------------------------------------------------------
    /*
    void getevtitem(name from) {
        itemevt_controller.getevtitem(from);
    }

    void addevtitem(uint32_t key, uint32_t code, uint32_t from, uint32_t day) {
        require_auth(self);
        itemevt_controller.addevtitem(key, code, from, day);
    }
    */

    // eosio.token recipient
    // memo description spec
    // knt:{type}
    // mw:{type}
    // item:{cid}
    // mat:{cid}
    // ivn:item
    // ivn:mat
    // eosknights:material-sale
    //-------------------------------------------------------------------------
    [[eosio::on_notify("eosio.token::transfer")]]
    void transfer(name from, name to, asset quantity, std::string memo) {
        system_controller.eosiotoken_transfer(from, to, quantity, memo, [&](const auto &ad) {
            if (ad.action.size() == 0) {
                return;
            }

            if (ad.action == ta_knt) {
                int type = atoi(ad.param.c_str());
                knight_controller.hireknight(ad.from, type, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_knight);
            } else if (ad.action == ta_mw) {
                int pid = atoi(ad.param.c_str());
                player_controller.buymp(ad.from, pid, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_mp);
            } else if (ad.action == ta_dmw) {
                auto info = require_season_open();
                assert_true(info.opt_no_dmw == false, "can not buy dmw this mode");
                int pid = atoi(ad.param.c_str());
                splayer_controller.buymp(ad.from, pid, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_dmw);
                season_controller.add_revenue(ad.quantity);
            } else if (ad.action == ta_item) {
                asset tax = market_controller.buyitem(ad.from, ad, &item_controller, ig_count);
                admin_controller.add_revenue(tax, rv_item_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_item_season) {
                auto info = require_season_open();
                asset tax = market_controller.buyitem(ad.from, ad, &sitem_controller, info.opt_item_shop);
                admin_controller.add_revenue(tax, rv_item_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_mat) {
                asset tax = market_controller.buymat(ad.from, ad, &material_controller, ig_count);
                admin_controller.add_revenue(tax, rv_material_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_mat_season) {
                auto info = require_season_open();
                asset tax = market_controller.buymat(ad.from, ad, &smaterial_controller, info.opt_mat_shop);
                admin_controller.add_revenue(tax, rv_material_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_skin) {
                asset tax = skin_controller.skbuy(ad.from, ad);
                admin_controller.add_revenue(tax, rv_skin);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_ivn) {
                if (ad.param == tp_item) {
                    system_controller.itemivnup(ad.from, ad.quantity);
                    admin_controller.add_revenue(ad.quantity, rv_item_iventory_up);
                } else if (ad.param == tp_mat) {
                    system_controller.mativnup(ad.from, ad.quantity);
                    admin_controller.add_revenue(ad.quantity, rv_mat_iventory_up);
                } else {
                    assert_true(false, "invalid inventory");
                }
            } else {
                assert_true(false, "invalid transfer");
            }

            admin_controller.autodividend();
        });
    }

    /*
    // Temporary implementation
    // Delete all data form chain. When table scheme is changed, all data 
    // should be cleaned. It's temporary api for development. And this method 
    // will be removed before launching.
    //-------------------------------------------------------------------------
    template<typename T>
    void clr(name from) {
        require_auth(self);

        T table(self, from);
        auto iter = table.begin();
        while (iter != table.cend()) {
            iter = table.erase(iter);
        }
    }
    */
};
