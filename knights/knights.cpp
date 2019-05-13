#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <cmath>

#define MAINTENANCE 0
#define USE_DEFERRED 1

using eosio::key256;
using eosio::indexed_by;
using eosio::const_mem_fun;
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
#include "contract/itemevt_control.hpp"
#include "contract/skin_control.hpp"
//#include "table/admin/novaevt.hpp"
//#include "contract/novaevt_control.hpp"

class knights : public eosio::contract, public control_base {
private:
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
    itemevt_control itemevt_controller; 
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
    knights(account_name self)
    : eosio::contract(self)
    , admin_controller(self)
    , saleslog_controller(_self)
    , player_controller(_self, saleslog_controller)
    , splayer_controller(_self, saleslog_controller)
    , system_controller(_self, player_controller, saleslog_controller, admin_controller)
    , material_controller(_self, system_controller, player_controller)
    , smaterial_controller(_self, system_controller, splayer_controller)
    , item_controller(_self, system_controller, player_controller, material_controller)
    , sitem_controller(_self, system_controller, splayer_controller, smaterial_controller)
    , pet_controller(_self, system_controller, player_controller, material_controller)
    , spet_controller(_self, system_controller, splayer_controller, smaterial_controller)
    , knight_controller(_self, system_controller, player_controller, material_controller, item_controller, pet_controller, saleslog_controller)
    , sknight_controller(_self, system_controller, splayer_controller, smaterial_controller, sitem_controller, spet_controller)
    , market_controller(_self, system_controller, player_controller, material_controller, item_controller, saleslog_controller, knight_controller)
    , cquest_controller(_self, item_controller, system_controller, admin_controller)
//    , dquest_controller(_self, item_controller, system_controller, admin_controller)
    , season_controller(_self, system_controller, admin_controller, sknight_controller, sitem_controller)
    , dungeon_controller(_self, system_controller, player_controller, material_controller, knight_controller/*, dquest_controller*/)
    , itemevt_controller(_self, system_controller, player_controller, item_controller)
    , skin_controller(_self, system_controller, saleslog_controller) {
    }

    // player related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void signup(name from) {
        require_auth(from);
        system_controller.signup(from);
        knight_controller.new_free_knight(from);
    }

    /// @abi action
    void signupbt(name from) {
        require_auth(N(bastetbastet));
        system_controller.signup(from);
        knight_controller.new_free_knight(from);
    }

    /// @abi action
    void referral(name from, name to) {
        system_controller.referral(from, to);
    }

    /// @abi action
    void getgift(name from, int16_t no) {
        system_controller.getgift(from, no);
    }

    /// @abi action
    void addgift(uint16_t no, uint8_t type, uint16_t amount, uint32_t to) {
        system_controller.addgift(no, type, amount, to);
    }

    /// @abi action
    void addcomment(name from, const std::string& message, const std::string& link) {
        system_controller.addcomment(from, message, link);
    }

    /// @abi action
    void reportofs(name from, name to) {
        system_controller.reportofs(from, to);
    }

    /// @abi action
    void addblackcmt(name to) {
        system_controller.addblackcmt(to);
    }

    // season related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void addseason(bool add, const seasoninfo &info) {
        season_controller.addseason(add, info);
    }

    /// @abi action
    void joinseason(name from) {
        season_controller.joinseason(from);
    }

    /// @abi action
    void seasonreward(name from, uint32_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        season_controller.seasonreward(from, id);
    }

    /// @abi action
    void submitsq(name from, int32_t season, int32_t id, uint32_t block, uint32_t checksum) {
        require_season(season);
        auto knt = season_controller.submitsq(from, id);
        if (knt > 0) {
            sknight_controller.refresh_stat(from, knt);
        }
    }

    void require_season(uint32_t season) {
        season_table table(_self, _self);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto iter = --table.cend();
        assert_true(iter->id == season, "invalid season");
        assert_true(iter->info.is_in(time_util::now()), "not in season period");
    }

    void require_season_open() {
        season_table table(_self, _self);
        assert_true(table.cbegin() != table.cend(), "no season yet");
        auto iter = --table.cend();
        auto now = time_util::now();
        assert_true(iter->info.is_in(now), "not in season period");
    }

    // cquest related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void addcquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        cquest_controller.addcquest(id, sponsor, start, duration);
    }

    // void removecquest(uint32_t id, bool force) {
    //    // it only available there is no user's record
    //    cquest_controller.removecquest(id, force);
    //}

    /// @abi action
    void updatesubq(uint32_t id, const std::vector<cquestdetail>& details) {
        cquest_controller.updatesubq(id, details);
    }

    /// @abi action
    void submitcquest(name from, uint32_t cquest_id, uint8_t no, uint32_t item_id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        cquest_controller.submitcquest(from, cquest_id, no, item_id);
    }

    /// @abi action
    void divcquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        cquest_controller.divcquest(id, no, from, count);
    }

    // dquest related actions
    //-------------------------------------------------------------------------
    /*
    /// @abi action
    void adddquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        dquest_controller.adddquest(id, sponsor, start, duration);
    }

    //void removedquest(uint32_t id, bool force) {
    //    // it only available there is no user's record
    //    dquest_controller.removedquest(id, force);
    //}

    /// @abi action
    void updatedsubq(uint32_t id, const std::vector<dquestdetail>& details) {
        dquest_controller.updatedsubq(id, details);
    }

    /// @abi action
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

    /// @abi action
    void lvupknight3(name from, uint32_t season, uint8_t type) {
        get_knight_ctl(season)->lvupknight(from, type);
    }

    /// @abi action
    void rebirth3(name from, uint32_t season, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_knight_ctl(season)->rebirth(from, season, checksum, true);
    }

    /// @abi action
    void rebirth3i(name from, uint32_t season, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_knight_ctl(season)->rebirth(from, season, checksum, false);
    }

    /// @abi action
    void equip3(name from, uint32_t season, uint8_t knight, uint32_t id) {
        get_knight_ctl(season)->equip(from, knight, id);
    }

    /// @abi action
    void detach3(name from, uint32_t season, uint32_t id) {
        get_knight_ctl(season)->detach(from, id);
    }

    /// @abi action
    void setkntstage(name from, uint8_t stage) {
        knight_controller.setkntstage(from, stage);
    }

    /// @abi action
    void skillup(name from, uint8_t knt, uint16_t id) {
        knight_controller.skillup(from, knt, id);
    }

    /// @abi action
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

    /// @abi action
    void removemat3(name from, uint32_t season, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_material_ctl(season)->remove(from, mat_ids);
    }

    /// @abi action
    void alchemist(name from, uint32_t grade, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        material_controller.alchemist(from, grade, mat_ids, checksum, true);
    }

    /// @abi action
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

    /// @abi action
    void craft3(name from, uint32_t season, uint16_t code, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        get_item_ctl(season)->craft(from, season, code, mat_ids, checksum, true);
    }

    /// @abi action
    void craft3i(name from, uint32_t season, uint16_t code, const std::vector<uint32_t>& mat_ids, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_item_ctl(season)->craft(from, season, code, mat_ids, checksum, false);
    }

    /// @abi action
    void itemlvup3(name from, uint32_t season, uint32_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        int8_t knight = get_item_ctl(season)->itemlvup(from, season, id, checksum, true);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    /// @abi action
    void itemlvup3i(name from, uint32_t season, uint32_t id, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        int8_t knight = get_item_ctl(season)->itemlvup(from, season, id, checksum, false);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    /// @abi action
    void removeitem3(name from, uint32_t season, const std::vector<uint32_t>& item_ids) {
        get_item_ctl(season)->remove(from, item_ids);
    }

    /// @abi action
    void itemmerge3(name from, uint32_t season, uint32_t id, const std::vector<uint32_t> &ingredient) {
        get_item_ctl(season)->itemmerge(from, id, ingredient);
    }

    // pet related actions
    //-------------------------------------------------------------------------
    pet_control_actions* get_pet_ctl(uint32_t season) {
        if (season == 0) {
            return &pet_controller;
        }

        require_season(season);
        return &spet_controller;
    }

    /// @abi action
    void petgacha3(name from, uint32_t season, uint16_t type, uint8_t count, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() > 0, "hire knight first!");
        get_pet_ctl(season)->petgacha(from, season, type, count, checksum, true);
    }

    /// @abi action
    void petgacha3i(name from, uint32_t season, uint16_t type, uint8_t count, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        get_pet_ctl(season)->petgacha(from, season, type, count, checksum, false);
    }

    /// @abi action
    void petlvup3(name from, uint32_t season, uint16_t code) {
        int8_t knight = get_pet_ctl(season)->petlvup(from, code);
        if (knight > 0) {
            get_knight_ctl(season)->refresh_stat(from, knight);
        }
    }

    /// @abi action
    void pattach3(name from, uint32_t season, uint16_t code, uint8_t knight) {
        get_pet_ctl(season)->pattach(from, code, knight);
        get_knight_ctl(season)->refresh_stat(from, knight);
    }

    /// @abi action
    void pexpstart2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        auto knight_max_level = knight_controller.get_knight_max_level(from);
        pet_controller.pexpstart(from, code, knight_max_level);
    }

    /// @abi action
    void pexpreturn2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        pet_controller.pexpreturn(from, code, checksum, true);
    }

    /// @abi action
    void pexpreturn2i(name from, uint16_t code, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        pet_controller.pexpreturn(from, code, checksum, false);
    }

    // market related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void sellitem2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.sellitem(from, id, price);
    }

    /// @abi action
    void ccsellitem2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellitem(from, id);
    }

    /// @abi action
    void sellmat2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.sellmat(from, id, price);
    }

    /// @abi action
    void ccsellmat2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellmat(from, id);
    }

    // dungeon related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void dgtcraft(name from, uint16_t code, const std::vector<uint32_t> &mat_ids, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgtcraft(from, code, mat_ids);
    }

    /// @abi action
    void dgfreetk2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgfreetk(from, code);
    }

    /// @abi action
    void dgenter(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgenter(from, code);
    }

    /// @abi action
    void dgclear(name from, uint16_t code, const std::vector<uint32_t> orders, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgclear(from, code, orders, checksum, true, false);
    }

    /// @abi action
    void dgcleari(name from, uint16_t code, const std::vector<uint32_t> orders, uint32_t checksum) {
        system_controller.set_last_checksum(checksum);
        dungeon_controller.dgclear(from, code, orders, checksum, false, false);
    }

    /// @abi action
    void dgleave(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        system_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgleave(from, code);
    }    

    // skin related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void skissue(uint16_t code, uint32_t count, asset price) {
        skin_controller.skissue(code, count, price);
    }

    /// @abi action
    void sksell(name from, uint32_t cid, asset price) {
        skin_controller.sksell(from, cid, price);
    }

    /// @abi action
    void skcsell(name from, uint32_t cid) {
        skin_controller.skcsell(from, cid);
    }

    /// @abi action
    void skwear(name from, uint32_t knt, uint32_t cid) {
        skin_controller.skwear(from, knt, cid);
    }

    // rule related actions
    //-------------------------------------------------------------------------
    /*
    /// @abi action
    void civnprice(const std::vector<rivnprice> &rules, bool truncate) {
        rule_controller<rivnprice, rivnprice_table> controller(_self, N(ivnprice));
        controller.create_rules(rules, truncate);
    }
    /*/
    /// @abi action
    /*
    void cknt(const std::vector<rknt> &rules, bool truncate) {
        rule_controller<rknt, rknt_table> controller(_self, N(knt));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void ckntlv(const std::vector<rkntlv> &rules, bool truncate) {
        rule_controller<rkntlv, rkntlv_table> controller(_self, N(kntlv));
        controller.create_rules(rules, truncate);
    }
    */
    /*
    /// @abi action
    void ckntprice(const std::vector<rkntprice> &rules, bool truncate) {
        rule_controller<rkntprice, rkntprice_table> controller(_self, N(kntprice));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void ckntskills(const std::vector<rkntskills> &rules, bool truncate) {
        rule_controller<rkntskills, rkntskills_table> controller(_self, N(kntskills));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cstage(const std::vector<rstage> &rules, bool truncate) {
        rule_controller<rstage, rstage_table> controller(_self, N(stage));
        controller.create_rules(rules, truncate);
    }
    */
    /// @abi action
    void cvariable(const std::vector<rvariable> &rules, bool truncate) {
        rule_controller<rvariable, rvariable_table> controller(_self, N(variable));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void citem(const std::vector<ritem> &rules, bool truncate) {
        rule_controller<ritem, ritem_table> controller(_self, N(item));
        controller.create_rules(rules, truncate);
    }

    /*
    /// @abi action
    void citemlv(const std::vector<ritemlv> &rules, bool truncate) {
        rule_controller<ritemlv, ritemlv_table> controller(_self, N(itemlv));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void citemset(const std::vector<ritemset> &rules, bool truncate) {
        rule_controller<ritemset, ritemset_table> controller(_self, N(itemset));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cmaterial(const std::vector<rmaterial> &rules, bool truncate) {
        rule_controller<rmaterial, rmaterial_table> controller(_self, N(material));
        controller.create_rules(rules, truncate);
    }
    
    /// @abi action
    void cpet(const std::vector<rpet> &rules, bool truncate) {
        rule_controller<rpet, rpet_table> controller(_self, N(pet));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cpetlv(const std::vector<rpetlv> &rules, bool truncate) {
        rule_controller<rpetlv, rpetlv_table> controller(_self, N(petlv));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cpetexp(const std::vector<rpetexp> &rules, bool truncate) {
        rule_controller<rpetexp, rpetexp_table> controller(_self, N(petexp));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cmpgoods(const std::vector<rmpgoods> &rules, bool truncate) {
        rule_controller<rmpgoods, rmpgoods_table> controller(_self, N(mpgoods));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cdungeon(const std::vector<rdungeon> &rules, bool truncate) {
        rule_controller<rdungeon, rdungeon_table> controller(_self, N(dungeon));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cdgticket(const std::vector<rdgticket> &rules, bool truncate) {
        rule_controller<rdgticket, rdgticket_table> controller;(_self, N(dgticket));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cmobs(const std::vector<rmobs> &rules, bool truncate) {
        rule_controller<rmobs, rmobs_table> controller(_self, N(mobs));
        controller.create_rules(rules, truncate);
    }

    /// @abi action
    void cmobskills(const std::vector<rmobskills> &rules, bool truncate) {
        rule_controller<rmobskills, rmobskills_table> controller(_self, N(mobskills));
        controller.create_rules(rules, truncate);
    }
    */
   
    /// @abi action
    void trule(name table, uint16_t size) {
        if (table == N(ivnprice)) {
            //rule_controller<rivnprice, rivnprice_table> controller(_self, N(ivnprice));
            //controller.truncate_rules(size);
        } else if (table == N(knt)) {
            //rule_controller<rknt, rknt_table> controller(_self, N(knt));
            //controller.truncate_rules(size);
        } else if (table == N(kntlv)) {
            //rule_controller<rkntlv, rkntlv_table> controller(_self, N(kntlv));
            //controller.truncate_rules(size);
        } else if (table == N(kntprice)) {
            //rule_controller<rkntprice, rkntprice_table> controller(_self, N(kntprice));
            //controller.truncate_rules(size);
        } else if (table == N(kntskills)) {
            //rule_controller<rkntskills, rkntskills_table> controller(_self, N(kntskills));
            //controller.truncate_rules(size);
        } else if (table == N(stage)) {
            //rule_controller<rstage, rstage_table> controller(_self, N(stage));
            //controller.truncate_rules(size);
        } else if (table == N(variable)) {
            rule_controller<rvariable, rvariable_table> controller(_self, N(variable));
            controller.truncate_rules(size);
        } else if (table == N(item)) {
            rule_controller<ritem, ritem_table> controller(_self, N(item));
            controller.truncate_rules(size);
        } else if (table == N(itemlv)) {
            //rule_controller<ritemlv, ritemlv_table> controller(_self, N(itemlv));
            //controller.truncate_rules(size);
        } else if (table == N(itemset)) {
            //rule_controller<ritemset, ritemset_table> controller(_self, N(itemset));
            //controller.truncate_rules(size);
        } else if (table == N(material)) {
            //rule_controller<rmaterial, rmaterial_table> controller(_self, N(material));
            //controller.truncate_rules(size);
        } else if (table == N(pet)) {
            //rule_controller<rpet, rpet_table> controller(_self, N(pet));
            //controller.truncate_rules(size);
        } else if (table == N(petlv)) {
            //rule_controller<rpetlv, rpetlv_table> controller(_self, N(petlv));
            //controller.truncate_rules(size);
        } else if (table == N(petexp)) {
            //rule_controller<rpetexp, rpetexp_table> controller(_self, N(petexp));
            //controller.truncate_rules(size);
        } else if (table == N(mpgoods)) {
            //rule_controller<rmpgoods, rmpgoods_table> controller(_self, N(mpgoods));
            //controller.truncate_rules(size);
        } else if (table == N(dungeon)) {
            //rule_controller<rdungeon, rdungeon_table> controller(_self, N(dungeon));
            //controller.truncate_rules(size);
        } else if (table == N(dgticket)) {
            //rule_controller<rdgticket, rdgticket_table> controller;(_self, N(dgticket));
            //controller.truncate_rules(size);
        } else if (table == N(mobs)) {
            //rule_controller<rmobs, rmobs_table> controller(_self, N(mobs));
            //controller.truncate_rules(size);
        } else if (table == N(mobskills)) {
            //rule_controller<rmobskills, rmobskills_table> controller(_self, N(mobskills));
            //controller.truncate_rules(size);
        } else {
            eosio_assert(0, "could not find table");
        }
    }

    // admin related actions
    //-------------------------------------------------------------------------
    // void setpause(uint8_t pid) {
    //    admin_controller.setpause(pid);
    // }

    void setcoo(name coo) {
        admin_controller.setcoo(coo);
    }

    /// @abi action
    void regsholder(name holder, uint16_t share) {
        admin_controller.regsholder(holder, share);
    }

    /// @abi action
    void dividend(asset amount) {
        require_auth(_self);
        admin_controller.dividend(amount);
    }

    // etc actions
    //-------------------------------------------------------------------------
    void getevtitem(name from) {
        itemevt_controller.getevtitem(from);
    }

    void addevtitem(uint32_t key, uint32_t code, uint32_t from, uint32_t day) {
        require_auth(_self);
        itemevt_controller.addevtitem(key, code, from, day);
    }

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
    void transfer(uint64_t sender, uint64_t receiver) {
        system_controller.eosiotoken_transfer(sender, receiver, [&](const auto &ad) {
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
                require_season_open();
                int pid = atoi(ad.param.c_str());
                splayer_controller.buymp(ad.from, pid, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_dmw);
                season_controller.add_revenue(ad.quantity);
            } else if (ad.action == ta_item) {
                asset tax = market_controller.buyitem(ad.from, ad, &item_controller);
                admin_controller.add_revenue(tax, rv_item_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_item_season) {
                require_season_open();
                asset tax = market_controller.buyitem(ad.from, ad, &sitem_controller);
                admin_controller.add_revenue(tax, rv_item_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_mat) {
                asset tax = market_controller.buymat(ad.from, ad, &material_controller);
                admin_controller.add_revenue(tax, rv_material_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_mat_season) {
                require_season_open();
                asset tax = market_controller.buymat(ad.from, ad, &smaterial_controller);
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
    /// @abi action
    void clrall() {
        clr<pet_table>(to_name(_self));
        clr<item_table>(to_name(_self));
        clr<material_table>(to_name(_self));
        clr<knight_table>(to_name(_self));
        clr<player_table>(to_name(_self));
        clr<adminstate_table>(to_name(_self));
        clr<item4sale_table>(to_name(_self));
        clr<mat4sale_table>(to_name(_self));
        clr<revenue_table>(to_name(_self));
        clr<revenuedt_table>(to_name(_self));
        clr<stockholder_table>(to_name(_self));
        clr<dividendlog_table>(to_name(_self));
        clr<expenseslog_table>(to_name(_self));
        clr<rversion_table>(to_name(_self));
    }

    template<typename T>
    void clr(name from) {
        require_auth(_self);

        T table(_self, from);
        auto iter = table.begin();
        while (iter != table.cend()) {
            iter = table.erase(iter);
        }
    }
    */
};

#undef EOSIO_ABI

#define EOSIO_ABI( TYPE, MEMBERS ) \
extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
        auto self = receiver; \
        TYPE thiscontract( self ); \
        if (MAINTENANCE == 1) { \
            require_auth(self); \
        }\
        if( action == N(onerror)) { \
            eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
        } \
        if( code == self ) { \
            if (action != N(transfer)) {\
                switch( action ) { \
                    EOSIO_API( TYPE, MEMBERS ) \
                } \
            }\
        } \
        else if (code == N(eosio.token) && action == N(transfer) ) {\
            execute_action(&thiscontract, &knights::transfer);\
        }\
    } \
}

// 
// 

EOSIO_ABI(knights, (signup) (signupbt) (referral) (getgift) (addcomment) (addblackcmt) (reportofs) (addseason) (joinseason) (seasonreward) (submitsq) (addgift) (addcquest) (updatesubq) (submitcquest) (divcquest) (setkntstage) (lvupknight3) (rebirth3) (rebirth3i) (equip3) (detach3) (alchemist) (alchemisti) (removemat3) (skillup) (skillreset) (craft3) (craft3i) (itemlvup3) (itemlvup3i) (removeitem3) (itemmerge3) (sellitem2) (ccsellitem2) (sellmat2) (ccsellmat2) (petgacha3) (petgacha3i) (petlvup3) (pattach3) (pexpstart2) (pexpreturn2i) (pexpreturn2) (dgtcraft) (dgfreetk2) (dgenter) (dgclear) (dgcleari) (dgleave) (skissue) (sksell) (skcsell) (skwear) (cvariable) (citem) (trule) (setcoo) (regsholder) (dividend) (getevtitem) (addevtitem) (transfer) ) // (clrall)
// (civnprice) (cknt) (ckntlv) (ckntprice) (ckntskills) (cstage) (cvariable) (citem) (citemlv) (citemset) (cmaterial) (cpet) (cpetlv) (cpetexp) (cdungeon) (cdgticket) (cmobs) (cmobskills) (cpet) (cpetlv) (cpetexp) (cmpgoods) 
// (removecquest) (removedquest) (setpause) 
// (adddquest) (updatedsubq) (divdquest) 
