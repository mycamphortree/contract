/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <eosio.token/eosio.token.hpp>
//#include <eosiolib/

#include <string>

using namespace eosio;

namespace eosiosystem {
   class system_contract;
}

namespace xiaomiantuan {

   using std::string;

   class token : public contract {
      public:
         token( account_name self ):contract(self){}

         void create( account_name issuer,
                      asset        maximum_supply);

         void issue( account_name to, asset quantity, string memo );

         void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );
      

        void quick_transfer( account_name from, account_name to, asset quantity, string memo );

      private:

         ///@abi table accounts i64
         struct account {
            account_name owner;
            asset    balance;

            uint64_t primary_key()const { return owner; }
            double   by_balance()const    { return balance.amount ? -balance.amount : balance.amount;  }
            EOSLIB_SERIALIZE( account, (owner)(balance))
         };



         ///@abi table stats i64
         struct currency_stats {
            asset          supply;
            asset          max_supply;
            account_name   issuer;

            uint64_t primary_key()const { return supply.symbol.name(); }
         };

    
         typedef eosio::multi_index<N(accounts) , account ,indexed_by<N(accountsbal), const_mem_fun<account, double, &account::by_balance> >
                                    > accounts;
         typedef eosio::multi_index<N(stat), currency_stats> stats;
         void bonus(int64_t in);
         void sub_balance( account_name owner, asset value );
         void add_balance( account_name owner, asset value, account_name ram_payer );


      public:
         struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            string        memo;
         };
   };



} /// namespace xiaomiantuan
