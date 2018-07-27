#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <cmath>

using eosio::key256;
using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::name;

#include "table/user/player.hpp"
#include "table/user/knight.hpp"
#include "table/user/material.hpp"
#include "table/user/mat4sale.hpp"
#include "table/user/item.hpp"
#include "table/user/item4sale.hpp"
#include "table/user/pet.hpp"
#include "table/user/revenue.hpp"
#include "table/rule/rivnprice.hpp"
#include "table/rule/rkntlv.hpp"
#include "table/rule/rkntprice.hpp"
#include "table/rule/rknt.hpp"
#include "table/rule/rstage.hpp"
#include "table/rule/rvariable.hpp"
#include "table/rule/ritem.hpp"
#include "table/rule/ritemlv.hpp"
#include "table/rule/rmaterial.hpp"
#include "table/rule/rpet.hpp"
#include "table/rule/rpetlv.hpp"
#include "table/rule/rmpgoods.hpp"
#include "table/outchain/knight_stats.hpp"
#include "table/outchain/transfer_action.hpp"
#include "table/token/st_transfer.hpp"
#include "table/token/airgrab.hpp"
#include "table/admin/adminstate.hpp"
#include "table/admin/airgrabstate.hpp"
#include "table/admin/revenuedt.hpp"
#include "table/admin/stockholder.hpp"
#include "table/admin/dividendlog.hpp"
#include "table/admin/expenseslog.hpp"
#include "table/admin/rversion.hpp"
#include "table/admin/marketpid.hpp"
#include "util/random_gen.hpp"
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

random_gen random_gen::instance;

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
    , player_controller(_self, saleslog_controller, admin_controller)
    , material_controller(_self, player_controller)
    , item_controller(_self, material_controller, player_controller, saleslog_controller)
    , pet_controller(_self, player_controller, saleslog_controller)
    , knight_controller(_self, material_controller, item_controller, pet_controller, player_controller, saleslog_controller)
    , market_controller(_self, material_controller, item_controller, player_controller, saleslog_controller, knight_controller)
    , powder_controller(_self, player_controller, saleslog_controller) {
    }

    // player related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void signup(name user) {
        player_controller.signup(user);
    }

    // knight related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void lvupknight(name from, uint8_t type) {
        knight_controller.lvupknight(from, type);
    }

    /// @abi action
    void rebirth(name from) {
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

    // material related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void removemat(name from, const std::vector<uint32_t>& mat_ids) {
        material_controller.remove(from, mat_ids);
    }

    // item related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void craft(name from, uint16_t code, const std::vector<uint32_t>& mat_ids) {
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
    void petgacha(name from, uint16_t type, uint8_t count) {
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

    // market related actions
    //-------------------------------------------------------------------------
    /// @abi action
    void sellitem(name from, uint64_t id, asset price) {
        market_controller.sellitem(from, id, price);
    }

    /// @abi action
    void ccsellitem(name from, uint64_t id) {
        market_controller.ccsellitem(from, id);
    }

    /// @abi action
    void sellmat(name from, uint64_t id, asset price) {
        market_controller.sellmat(from, id, price);
    }

    /// @abi action
    void ccsellmat(name from, uint64_t id) {
        market_controller.ccsellmat(from, id);
    }

    /// @abi action
    void isuadmats(const std::vector<uint16_t> &matids, const std::vector<asset> &prices) {
        auto coo = admin_controller.get_coo();
        market_controller.isuadmats(matids, prices, coo);
    }

    /// @abi action
    void rmadmats(const std::vector<uint16_t> &ids) {
        auto coo = admin_controller.get_coo();
        market_controller.rmadmats(ids, coo);
    }

    // rule ralated actions
    //-------------------------------------------------------------------------
    /// @abi action
    void civnprice(const std::vector<rivnprice> &rules) {
        player_controller.get_inventory_price_rule().create_rules(rules);
    }

    /// @abi action
    void cknt(const std::vector<rknt> &rules) {
        knight_controller.get_knight_rule_controller().create_rules(rules);
    }

    /// @abi action
    void ckntlv(const std::vector<rkntlv> &rules) {
        knight_controller.get_knight_level_rule_controller().create_rules(rules);
    }

    /// @abi action
    void ckntprice(const std::vector<rkntprice> &rules) {
        knight_controller.get_knight_price_rule_controller().create_rules(rules);
    }

    /// @abi action
    void cstage(const std::vector<rstage> &rules) {
        knight_controller.get_stage_rule_controller().create_rules(rules);
    }

    /// @abi action
    void cvariable(const std::vector<rvariable> &rules) {
        variable_controller.get_rvariable_rule().create_rules(rules);
    }

    /// @abi action
    void citem(const std::vector<ritem> &rules) {
        item_controller.get_ritem_rule().create_rules(rules);
    }

    /// @abi action
    void citemlv(const std::vector<ritemlv> &rules) {
        item_controller.get_ritemlv_rule().create_rules(rules);
    }

    /// @abi action
    void cmaterial(const std::vector<rmaterial> &rules) {
        material_controller.get_rmaterial_rule().create_rules(rules);
    }

    /// @abi action
    void cpet(const std::vector<rpet> &rules) {
        pet_controller.get_pet_rule().create_rules(rules);
    }

    /// @abi action
    void cpetlv(const std::vector<rpetlv> &rules) {
        pet_controller.get_pet_level_rule().create_rules(rules);
    }

    /// @abi action
    void cmpgoods(const std::vector<rmpgoods> &rules) {
        powder_controller.get_mpgoods_rule().create_rules(rules);
    }

    /// @abi action
    void trule(name table) {
        if (table == N(ivnprice)) {
            player_controller.get_inventory_price_rule().truncate_rules();
        } else if (table == N(knt)) {
            knight_controller.get_knight_rule_controller().truncate_rules();
        } else if (table == N(kntlv)) {
            knight_controller.get_knight_level_rule_controller().truncate_rules();
        } else if (table == N(kntprice)) {
            knight_controller.get_knight_price_rule_controller().truncate_rules();
        } else if (table == N(stage)) {
            knight_controller.get_stage_rule_controller().truncate_rules();
        } else if (table == N(variable)) {
            variable_controller.get_rvariable_rule().truncate_rules();
        } else if (table == N(item)) {
            item_controller.get_ritem_rule().truncate_rules();
        } else if (table == N(itemlv)) {
            item_controller.get_ritemlv_rule().truncate_rules();
        } else if (table == N(material)) {
            material_controller.get_rmaterial_rule().truncate_rules();
        } else if (table == N(pet)) {
            pet_controller.get_pet_rule().truncate_rules();
        } else if (table == N(petlv)) {
            pet_controller.get_pet_level_rule().truncate_rules();
        } else if (table == N(mpgoods)) {
            powder_controller.get_mpgoods_rule().truncate_rules();
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
    void setairgrab(asset total) {
        admin_controller.setairgrab(total);
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
        auto transfer_data = eosio::unpack_action_data<st_transfer>();
        eosio_assert(transfer_data.quantity.is_valid(), "Invalid token transfer");
        eosio_assert(transfer_data.quantity.amount > 0, "Quantity must be positive");

        if (transfer_data.quantity.symbol == S(4, EOS)) {
            process_eos_transfer(transfer_data);
        } else if (transfer_data.quantity.symbol == S(4, BADA)) {
            process_bada_transfer(transfer_data);
        } else {
            assert_true(false, "only accepts EOS or BADA for transfer");
        }
    }

    void process_bada_transfer(const st_transfer &transfer_data) {
        player_controller.bada_transfer(transfer_data, [&](const auto &ad) {
            if (ad.action == ta_mw) {
                int pid = atoi(ad.param.c_str());
                powder_controller.buymp(ad.from, pid, ad.quantity);
                admin_controller.add_bada_revenue(transfer_data.quantity);
            } else {
                assert_true(false, "invalid transfer");
            }
        });
    }

    void process_eos_transfer(const st_transfer &transfer_data) {
        player_controller.eos_transfer(transfer_data, [&](const auto &ad) {
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
                uint64_t saleid = atoll(ad.param.c_str());
                asset tax = market_controller.buyitem(ad.from, saleid, ad.quantity);
                admin_controller.add_revenue(tax, rv_item_tax);
            } else if (ad.action == ta_mat) {
                uint64_t saleid = atoll(ad.param.c_str());
                asset tax = market_controller.buymat(ad.from, saleid, ad.quantity);
                admin_controller.add_revenue(tax, rv_material_tax);
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

    /// @abi action
    void claimbada(name from, uint8_t index) {
        player_controller.claimbada(from, index, knight_controller.get_knights(from));
    }

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
        clr<stockholder_table>(to_name(_self));
        clr<dividendlog_table>(to_name(_self));
        clr<expenseslog_table>(to_name(_self));
    }

    /// @abi action
    void rmdivid() {
        clr<dividendlog_table>(to_name(_self));
    }

    /// @abi action
    void rmmat4sale(uint32_t id) {
        mat4sale_table table(_self, _self);
        table.erase(table.find(id));
    }

    /// @abi action
    void rmplayer(name id) {
        player_table table(_self, _self);
        table.erase(table.find(id));
    }

    /// @abi action
    void setadmasset(asset revenue, asset dividend, asset expenses) {
        adminstate_table table(_self, _self);
        table.modify(table.begin(), _self, [&](auto &target) {
            target.revenue = revenue;
            target.dividend = dividend;
            target.expenses = expenses;
        });
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
};

#undef EOSIO_ABI

#define EOSIO_ABI( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      auto self = receiver; \
      if( code == self || code == N(eosio.token) || code == N(badatokenbnk) || action == N(onerror) ) { \
         TYPE thiscontract( self ); \
         switch( action ) { \
            EOSIO_API( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
}


EOSIO_ABI(knights, (signup) (lvupknight) (setkntstage) (rebirth) (removemat) (craft) (removeitem) (equip) (detach) (itemmerge) (itemlvup) (sellitem) (ccsellitem) (sellmat) (ccsellmat) (isuadmats) (rmadmats) (petgacha) (petlvup) (pattach) (civnprice) (cknt) (ckntlv) (ckntprice) (cstage) (cvariable) (citem) (citemlv) (cmaterial) (cpet) (cpetlv) (cmpgoods) (trule) (setpause) (setcoo) (setairgrab) (regsholder) (dividend) (claimbada) (transfer)       (clrall) (rmmat4sale) (rmplayer) (setadmasset) (rmdivid))