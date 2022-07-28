//
// Created by discens on 7/1/20.
//

#ifndef BITCOIN_VALIDATION_H
#define BITCOIN_VALIDATION_H
#include "amount.h"
#include "chain.h"
static const uint64_t DEFAULT_GAS_LIMIT_OP_CREATE=2500000; // very big limit will throw `insane fee`
static const uint64_t DEFAULT_GAS_LIMIT_OP_SEND=250000;
static const CAmount DEFAULT_GAS_PRICE=0.00000040*COIN;
static const CAmount MAX_RPC_GAS_PRICE=0.00000100*COIN;
static const uint64_t MINIMUM_GAS_LIMIT = 10000;
static const uint64_t MEMPOOL_MIN_GAS_LIMIT = 22000;

#endif //BITCOIN_VALIDATION_H
