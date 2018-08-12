/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "xiaomiantuan.hpp"


using namespace eosio;

#define MT_SYMBOL S(4,MT)
#define CORE_SYMBOL_TO_MT_SYMBOL 1

namespace xiaomiantuan {

void token::create( account_name issuer,
                    asset        maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym == MT_SYMBOL, "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}


void token::issue( account_name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    sub_balance( from, quantity );
    add_balance( to, quantity, from );
}

void token::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, _self);

   const auto& from = from_acnts.get( owner, "no balance object found" );
   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );


   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, owner, [&]( auto& a ) {
          a.owner = owner;
          a.balance -= value;
      });
   }
}

void token::add_balance( account_name owner, asset value, account_name ram_payer )
{
   accounts to_acnts( _self, _self );
   auto to = to_acnts.find( owner );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.owner = owner;
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.owner = owner;
        a.balance += value;
      });
   }
}

void token::bonus(int64_t in){
    symbol_type mts = MT_SYMBOL;
    stats statstable( _self, mts.name());
    accounts acnts( _self, _self );
    auto existing = statstable.find( mts.name() );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    int64_t sum = existing->supply.amount;
    eosio_assert(sum > 0 , "must have supply");
    auto idx = acnts.get_index<N(accountsbal)>();
    double temp = 0;
    int64_t send = 0;
    for ( auto it = idx.cbegin(); it != idx.cend(); ++it ) {
        temp = it->balance.amount * in * 1.0 / sum;
        send = int64_t(temp);
        if(send > 0){
            INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {_self,N(active)},
                                                          { _self, it->owner, asset(send,CORE_SYMBOL), std::string("报告股东，分红到了") } );
        }

    }
}

void token::quick_transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio_assert( from != to, "quick_transfer : cannot transfer to self" );
    if(from == _self){
        return;
    }
    eosio_assert(  to == _self, "must to xiaomiantuan contract");
    eosio_assert(  quantity.symbol == CORE_SYMBOL, "must use system coin");

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    // eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    if(memo == string("bonus")){
        bonus(quantity.amount);
        print("bonus");
        return;
    }



    asset mt_quantity(quantity.amount,MT_SYMBOL);
    mt_quantity.amount *= CORE_SYMBOL_TO_MT_SYMBOL;

    auto sym = mt_quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    eosio_assert( mt_quantity.is_valid(), "invalid mt_quantity" );
    eosio_assert( mt_quantity.amount > 0, "must issue positive mt_quantity" );

    eosio_assert( mt_quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( mt_quantity.amount <= st.max_supply.amount - st.supply.amount, "mt_quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += mt_quantity;
    });

    add_balance( st.issuer, mt_quantity, st.issuer );

    if( from != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, from, mt_quantity, memo} );
       INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {_self,N(active)},
                                                      { _self, N(mycamphor111), quantity, std::string("To the company") } );
    }

}


} /// namespace xiaomiantuan



#define XIAOMIANTUAN_EOSIO_ABI( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(action == N(transfer) && code == N(eosio.token)) { \
         TYPE thiscontract( self ); \
         eosio::execute_action( &thiscontract, &xiaomiantuan::token::quick_transfer); \
         return; \
      } \
      if(code == self || action == N(onerror) ) { \
         TYPE thiscontract( self ); \
         switch( action ) { \
            EOSIO_API( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
} \


XIAOMIANTUAN_EOSIO_ABI( xiaomiantuan::token, (create)(issue)(transfer) )
