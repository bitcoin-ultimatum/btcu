// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SCRIPT_SIGCACHE_H
#define BITCOIN_SCRIPT_SIGCACHE_H

#include "script/interpreter.h"

#include <vector>

class CPubKey;


namespace BTC {

   class SignatureCacheHasher
   {
   public:
      template <uint8_t hash_select>
      uint32_t operator()(const uint256& key) const
      {
         static_assert(hash_select <8, "SignatureCacheHasher only has 8 hashes available.");
         uint32_t u;
         std::memcpy(&u, key.begin()+4*hash_select, 4);
         return u;
      }
   };

class CachingTransactionSignatureChecker : public BTC::TransactionSignatureChecker
   {
   private:
      bool store;

   public:
      CachingTransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, bool storeIn, PrecomputedTransactionData& txdataIn) : TransactionSignatureChecker(txToIn, nInIn, amountIn, txdataIn), store(storeIn) {}

      bool VerifySignature(const std::vector<unsigned char>& vchSig, const CPubKey& vchPubKey, const uint256& sighash) const override;
   };
}

#endif // BITCOIN_SCRIPT_SIGCACHE_H
