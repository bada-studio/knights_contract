#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <cmath>

#define MAINTENANCE 0

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
#include "util/time_util.hpp"
#include "contract/control_base.hpp"
#include "contract/admin_control.hpp"
#include "contract/rule_controller.hpp"
#include "contract/saleslog_control.hpp"
#include "contract/variable_control.hpp"
#include "contract/player_control.hpp"
#include "contract/material_control.hpp"
#include "contract/item_control.hpp"
#include "contract/pet_control.hpp"
#include "contract/knight_control.hpp"
#include "contract/market_control.hpp"
#include "contract/powder_control.hpp"
#include "contract/cquest_control.hpp"
#include "contract/player_control.cpp"
#include "contract/dungeon_control.hpp"

class knights : public eosio::contract, public control_base {
private:
    // controls
    variable_control variable_controller;
    player_control player_controller;
    material_control material_controller;
    pet_control pet_controller;
    knight_control knight_controller;
    item_control item_controller;
    market_control market_controller;
    powder_control powder_controller;
    admin_control admin_controller;
    saleslog_control saleslog_controller;
    cquest_control cquest_controller;
    dungeon_control dungeon_controller;

    const char* ta_knt = "knt";
    const char* ta_mw = "mw";
    const char* ta_item = "item";
    const char* ta_mat = "mat";
    const char* ta_ivn = "ivn";
    const char* tp_item = "item";
    const char* tp_mat = "mat";
    const char* ta_eosknights = "eosknights";
    const char* tp_mat_sale = "material-sale";

public:
    knights(account_name self)
    : eosio::contract(self)
    , admin_controller(self)
    , variable_controller(_self)
    , saleslog_controller(_self)
    , player_controller(_self, saleslog_controller, admin_controller, variable_controller)
    , material_controller(_self, player_controller)
    , item_controller(_self, material_controller, player_controller, saleslog_controller)
    , pet_controller(_self, player_controller, material_controller, saleslog_controller)
    , knight_controller(_self, material_controller, item_controller, pet_controller, player_controller, saleslog_controller)
    , market_controller(_self, material_controller, item_controller, player_controller, saleslog_controller, knight_controller)
    , powder_controller(_self, player_controller, saleslog_controller)
    , cquest_controller(_self, item_controller, player_controller, admin_controller)
    , dungeon_controller(_self, material_controller, player_controller, knight_controller) {
    }

    // player related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void signup(name from) {
        player_controller.signup(from);
    }

    /// @abi action
    void referral(name from, name to) {
        player_controller.referral(from, to);
    }

    /// @abi action
    void getgift(name from, int16_t no) {
        player_controller.getgift(from, no);
    }

    /// @abi action
    void addgift(uint16_t no, uint8_t type, uint16_t amount, uint32_t to) {
        player_controller.addgift(no, type, amount, to);
    }

    /// @abi action
    void shuffle(name from) {
        player_controller.shuffle(from);
    }

    // cquest related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void addcquest(uint32_t id, uint16_t sponsor, uint32_t start, uint32_t duration) {
        cquest_controller.addcquest(id, sponsor, start, duration);
    }

    /// @abi action
    void removecquest(uint32_t id, bool force) {
        // it only available there is no user's record
        cquest_controller.removecquest(id, force);
    }

    /// @abi action
    void updatesubq(uint32_t id, const std::vector<cquestdetail>& details) {
        cquest_controller.updatesubq(id, details);
    }

    /// @abi action
    void submitcquest(name from, uint32_t cquest_id, uint8_t no, uint32_t item_id, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        cquest_controller.submitcquest(from, cquest_id, no, item_id);
    }

    /// @abi action
    void divcquest(uint32_t id, uint8_t no, int16_t from, int16_t count) {
        cquest_controller.divcquest(id, no, from, count);
    }

    // knight related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void lvupknight(name from, uint8_t type) {
        knight_controller.lvupknight(from, type);
    }

    /// @abi action
    void rebirth2(name from, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        knight_controller.rebirth(from);
    }

    /// @abi action
    void setkntstage(name from, uint8_t stage) {
        knight_controller.setkntstage(from, stage);
    }

    /// @abi action
    void equip(name from, uint8_t knight, uint32_t id) {
        knight_controller.equip(from, knight, id);
    }

    /// @abi action
    void detach(name from, uint32_t id) {
        knight_controller.detach(from, id);
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
    /// @abi action
    void removemat2(name from, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        material_controller.remove(from, mat_ids);
    }

    // item related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void craft2(name from, uint16_t code, const std::vector<uint32_t>& mat_ids, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        item_controller.craft(from, code, mat_ids);
    }

    /// @abi action
    void removeitem(name from, const std::vector<uint32_t>& item_ids) {
        item_controller.remove(from, item_ids);
    }

    /// @abi action
    void itemmerge(name from, uint32_t id, const std::vector<uint32_t> &ingredient) {
        item_controller.itemmerge(from, id, ingredient);
    }

    /// @abi action
    void itemlvup(name from, uint32_t id) {
        int8_t knight = item_controller.itemlvup(from, id);
        if (knight > 0) {
            knight_controller.refresh_stat(from, knight);
        }
    }

    // pet related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void petgacha2(name from, uint16_t type, uint8_t count, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        auto &knights = knight_controller.get_knights(from);
        assert_true(knights.size() > 0, "hire knight first!");
        pet_controller.petgacha(from, type, count);
    }

    /// @abi action
    void petlvup(name from, uint16_t code) {
        int8_t knight = pet_controller.petlvup(from, code);
        if (knight > 0) {
            knight_controller.refresh_stat(from, knight);
        }
    }

    /// @abi action
    void pattach(name from, uint16_t code, uint8_t knight) {
        pet_controller.pattach(from, code, knight);
        knight_controller.refresh_stat(from, knight);
    }

    /// @abi action
    void pexpstart(name from, uint16_t code) {
        auto knight_max_level = knight_controller.get_knight_max_level(from);
        pet_controller.pexpstart(from, code, knight_max_level);
    }

    /// @abi action
    void pexpreturn(name from, uint16_t code) {
        pet_controller.pexpreturn(from, code);
    }

    /// @abi action
    void pexpreturn2(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        pet_controller.pexpreturn(from, code);
    }

    // market related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void sellitem2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        market_controller.sellitem(from, id, price);
    }

    /// @abi action
    void ccsellitem2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellitem(from, id);
    }

    /// @abi action
    void sellmat2(name from, uint64_t id, asset price, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        market_controller.sellmat(from, id, price);
    }

    /// @abi action
    void ccsellmat2(name from, uint64_t id, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        market_controller.ccsellmat(from, id);
    }

    // dungeon related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void dgtcraft(name from, uint16_t code, const std::vector<uint32_t> &mat_ids, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgtcraft(from, code, mat_ids);
    }

    /// @abi action
    void dgfreetk(name from, uint16_t code) {
        dungeon_controller.dgfreetk(from, code);
    }

    /// @abi action
    void dgenter(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgenter(from, code);
    }

    /// @abi action
    void dgclear(name from, uint16_t code, const std::vector<dgorder> orders, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgclear(from, code, orders);
    }

    /// @abi action
    void dgleave(name from, uint16_t code, uint32_t block, uint32_t checksum) {
        player_controller.checksum_gateway(from, block, checksum);
        dungeon_controller.dgleave(from, code);
    }    

    // rule related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void civnprice(const std::vector<rivnprice> &rules, bool truncate) {
        player_controller.get_inventory_price_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cknt(const std::vector<rknt> &rules, bool truncate) {
        knight_controller.get_knight_rule_controller().create_rules(rules, truncate);
    }

    /// @abi action
    void ckntlv(const std::vector<rkntlv> &rules, bool truncate) {
        knight_controller.get_knight_level_rule_controller().create_rules(rules, truncate);
    }

    /// @abi action
    void ckntprice(const std::vector<rkntprice> &rules, bool truncate) {
        knight_controller.get_knight_price_rule_controller().create_rules(rules, truncate);
    }

    /// @abi action
    void ckntskills(const std::vector<rkntskills> &rules, bool truncate) {
        knight_controller.get_knight_skill_rule_controller().create_rules(rules, truncate);
    }

    /// @abi action
    void cstage(const std::vector<rstage> &rules, bool truncate) {
        knight_controller.get_stage_rule_controller().create_rules(rules, truncate);
    }

    /// @abi action
    void cvariable(const std::vector<rvariable> &rules, bool truncate) {
        variable_controller.get_rvariable_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void citem(const std::vector<ritem> &rules, bool truncate) {
        item_controller.get_ritem_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void citemlv(const std::vector<ritemlv> &rules, bool truncate) {
        item_controller.get_ritemlv_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void citemset(const std::vector<ritemset> &rules, bool truncate) {
        item_controller.get_ritemset_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cmaterial(const std::vector<rmaterial> &rules, bool truncate) {
        material_controller.get_rmaterial_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cpet(const std::vector<rpet> &rules, bool truncate) {
        pet_controller.get_pet_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cpetlv(const std::vector<rpetlv> &rules, bool truncate) {
        pet_controller.get_pet_level_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cpetexp(const std::vector<rpetexp> &rules, bool truncate) {
        pet_controller.get_pet_exp_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cmpgoods(const std::vector<rmpgoods> &rules, bool truncate) {
        powder_controller.get_mpgoods_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cdungeon(const std::vector<rdungeon> &rules, bool truncate) {
        dungeon_controller.get_dungeon_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cdgticket(const std::vector<rdgticket> &rules, bool truncate) {
        dungeon_controller.get_dgticket_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cmobs(const std::vector<rmobs> &rules, bool truncate) {
        dungeon_controller.get_mobs_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void cmobskills(const std::vector<rmobskills> &rules, bool truncate) {
        dungeon_controller.get_mobskills_rule().create_rules(rules, truncate);
    }

    /// @abi action
    void trule(name table, uint16_t size) {
        if (table == N(ivnprice)) {
            player_controller.get_inventory_price_rule().truncate_rules(size);
        } else if (table == N(knt)) {
            knight_controller.get_knight_rule_controller().truncate_rules(size);
        } else if (table == N(kntlv)) {
            knight_controller.get_knight_level_rule_controller().truncate_rules(size);
        } else if (table == N(kntprice)) {
            knight_controller.get_knight_price_rule_controller().truncate_rules(size);
        } else if (table == N(kntskills)) {
            knight_controller.get_knight_skill_rule_controller().truncate_rules(size);
        } else if (table == N(stage)) {
            knight_controller.get_stage_rule_controller().truncate_rules(size);
        } else if (table == N(variable)) {
            variable_controller.get_rvariable_rule().truncate_rules(size);
        } else if (table == N(item)) {
            item_controller.get_ritem_rule().truncate_rules(size);
        } else if (table == N(itemlv)) {
            item_controller.get_ritemlv_rule().truncate_rules(size);
        } else if (table == N(itemset)) {
            item_controller.get_ritemset_rule().truncate_rules(size);
        } else if (table == N(material)) {
            material_controller.get_rmaterial_rule().truncate_rules(size);
        } else if (table == N(pet)) {
            pet_controller.get_pet_rule().truncate_rules(size);
        } else if (table == N(petlv)) {
            pet_controller.get_pet_level_rule().truncate_rules(size);
        } else if (table == N(petexp)) {
            pet_controller.get_pet_exp_rule().truncate_rules(size);
        } else if (table == N(mpgoods)) {
            powder_controller.get_mpgoods_rule().truncate_rules(size);
        } else if (table == N(dungeon)) {
            dungeon_controller.get_dungeon_rule().truncate_rules(size);
        } else if (table == N(dgticket)) {
            dungeon_controller.get_dgticket_rule().truncate_rules(size);
        } else if (table == N(mobs)) {
            dungeon_controller.get_mobs_rule().truncate_rules(size);
        } else if (table == N(mobskills)) {
            dungeon_controller.get_mobskills_rule().truncate_rules(size);
        } else {
            eosio_assert(0, "could not find table");
        }
    }

    // admin related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void setpause(uint8_t pid) {
        admin_controller.setpause(pid);
    }

    /// @abi action
    void setcoo(name coo) {
        admin_controller.setcoo(coo);
    }

    /// @abi action
    void regsholder(name holder, uint16_t share) {
        admin_controller.regsholder(holder, share);
    }

    /// @abi action
    void dividend(asset amount) {
        admin_controller.dividend(amount);
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
        player_controller.eosiotoken_transfer(sender, receiver, [&](const auto &ad) {
            if (ad.action.size() == 0) {
                return;
            }

            if (ad.action == ta_knt) {
                int type = atoi(ad.param.c_str());
                knight_controller.hireknight(ad.from, type, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_knight);
            } else if (ad.action == ta_mw) {
                int pid = atoi(ad.param.c_str());
                powder_controller.buymp(ad.from, pid, ad.quantity);
                admin_controller.add_revenue(ad.quantity, rv_mp);
            } else if (ad.action == ta_item) {
                asset tax = market_controller.buyitem(ad.from, ad);
                admin_controller.add_revenue(tax, rv_item_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_mat) {
                asset tax = market_controller.buymat(ad.from, ad);
                admin_controller.add_revenue(tax, rv_material_tax);
                admin_controller.add_tradingvol(ad.quantity);
            } else if (ad.action == ta_ivn) {
                if (ad.param == tp_item) {
                    player_controller.itemivnup(ad.from, ad.quantity);
                    admin_controller.add_revenue(ad.quantity, rv_item_iventory_up);
                } else if (ad.param == tp_mat) {
                    player_controller.mativnup(ad.from, ad.quantity);
                    admin_controller.add_revenue(ad.quantity, rv_mat_iventory_up);
                } else {
                    assert_true(false, "invalid inventory");
                }
                
            } else if (ad.action == ta_eosknights && ad.param == tp_mat_sale) {
                // coo material sales revenue
                admin_controller.add_revenue(ad.quantity, rv_coo_mat);
            } else {
                assert_true(false, "invalid transfer");
            }
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

EOSIO_ABI(knights, (signup) (referral) (getgift) (addgift) (shuffle) (addcquest) (removecquest) (updatesubq) (submitcquest) (divcquest) (lvupknight) (setkntstage) (rebirth2) (removemat2) (craft2) (removeitem) (equip) (detach) (skillup) (skillreset) (itemmerge) (itemlvup) (sellitem2) (ccsellitem2) (sellmat2) (ccsellmat2) (petgacha2) (petlvup) (pattach) (pexpstart) (pexpreturn) (pexpreturn2) (dgtcraft) (dgfreetk) (dgenter) (dgclear) (dgleave) (civnprice) (cknt) (ckntlv) (ckntprice) (ckntskills) (cstage) (cvariable) (citem) (citemlv) (citemset) (cmaterial) (cpet) (cpetlv) (cpetexp) (cmpgoods) (cdungeon) (cdgticket) (cmobs) (cmobskills) (trule) (setpause) (setcoo) (regsholder) (dividend) (transfer) ) // (clrall)