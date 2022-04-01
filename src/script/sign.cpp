// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/sign.h"

#include "primitives/transaction.h"
#include "key.h"
#include "keystore.h"
#include "script/standard.h"
#include "uint256.h"
#include "util.h"



typedef std::vector<unsigned char> valtype;

bool Sign1(const CKeyID& address, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet, std::vector<valtype>& ret)
{
    CKey key;
    if (!keystore.GetKey(address, key))
        return false;

    std::vector<unsigned char> vchSig;
    if (!key.Sign(hash, vchSig))
        return false;
    vchSig.push_back((unsigned char)nHashType);
    ret.push_back(std::move(CScript(vchSig.begin(), vchSig.end())));
    scriptSigRet << vchSig;

    return true;
}

bool SignN(const std::vector<valtype>& multisigdata, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet, std::vector<valtype>& ret)
{
    int nSigned = 0;
    int nRequired = multisigdata.front()[0];
    ret.push_back(valtype());
    for (unsigned int i = 1; i < multisigdata.size()-1 && nSigned < nRequired; i++)
    {
        const valtype& pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
        {
            ++nSigned;
        }
    }
    return nSigned==nRequired;
}

/**
 * Sign scriptPubKey with private keys stored in keystore, given transaction hash and hash type.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey can't be signed),
 * unless whichTypeRet is TX_SCRIPTHASH, in which case scriptSigRet is the redemption script.
 * Returns false if scriptPubKey could not be completely satisfied.
 */
bool SignStep(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType,
              CScript& scriptSigRet, txnouttype& whichTypeRet, std::vector<valtype>& ret, bool fColdStake = false,
              bool fLeasing = false, bool fForceLeaserSign = false)
{
    scriptSigRet.clear();

    std::vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, whichTypeRet, vSolutions))
    {
        LogPrintf("*** solver solver failed \n");
        return false;
    }

    CKeyID keyID;
    CPubKey vch;
    switch (whichTypeRet)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
    case TX_WITNESS_UNKNOWN:
    {
        LogPrintf("*** null data \n");
        return false;
    }
    case TX_ZEROCOINMINT:
        return false;
    case TX_PUBKEY:
        keyID = CPubKey(vSolutions[0]).GetID();
        if(!Sign1(keyID, keystore, hash, nHashType, scriptSigRet, ret))
        {
            LogPrintf("*** Sign1 failed \n");
            return false;
        }
        return true;
    case TX_PUBKEYHASH:
        keyID = CKeyID(uint160(vSolutions[0]));
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
        {
            LogPrintf("*** solver failed to sign \n");
            return false;
        }
        else
        {
            if (!keystore.GetPubKey(keyID, vch))
                return error("%s : Unable to get public key from keyID", __func__);
            //ret.push_back(std::move( CScript(scriptSigRet.begin() + 1, scriptSigRet.end()) ));
            ret.push_back(ToByteVector(vch));
            scriptSigRet << ToByteVector(vch);
        }

        return true;
    case TX_SCRIPTHASH:
        return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);

    case TX_WITNESS_V0_KEYHASH:
        scriptSigRet = CScript(vSolutions[0].begin(), vSolutions[0].end());
        return true;

    case TX_WITNESS_V0_SCRIPTHASH: {
        uint160 h160;
        CRIPEMD160().Write(&vSolutions[0][0], vSolutions[0].size()).Finalize(h160.begin());
        return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);
    }

    case TX_MULTISIG:
        scriptSigRet << OP_0; // workaround CHECKMULTISIG bug
        return (SignN(vSolutions, keystore, hash, nHashType, scriptSigRet,ret));

    case TX_COLDSTAKE: {
        if (fColdStake) {
            // sign with the cold staker key
            keyID = CKeyID(uint160(vSolutions[0]));
        } else {
            // sign with the owner key
            keyID = CKeyID(uint160(vSolutions[1]));
        }
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
            return error("*** %s: failed to sign with the %s key.",
                    __func__, fColdStake ? "cold staker" : "owner");
        CPubKey vch;
        if (!keystore.GetPubKey(keyID, vch))
            return error("%s : Unable to get public key from keyID", __func__);
        scriptSigRet << (fColdStake ? (int)OP_TRUE : OP_FALSE) << ToByteVector(vch);
        return true;
      }
    case TX_LEASE_CLTV:
    case TX_LEASE: {
        if (fLeasing) {
            if (!fForceLeaserSign) {
                LogPrintf("*** solver met leaser, who can't sign \n");
                return false;
            }
            // sign with the leaser key
            keyID = (whichTypeRet == TX_LEASE) ? CKeyID(uint160(vSolutions[0])): CKeyID(uint160(vSolutions[1]));
        } else {
            // sign with the owner key
            keyID = (whichTypeRet == TX_LEASE) ? CKeyID(uint160(vSolutions[1])): CKeyID(uint160(vSolutions[2]));
        }
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet, ret))
            return error("*** %s: failed to sign with the %s key.",
                         __func__, fLeasing ? "leaser" : "owner");
        CPubKey vch;
        if (!keystore.GetPubKey(keyID, vch))
            return error("%s : Unable to get public key from keyID", __func__);
        scriptSigRet << (fLeasing ? (int)OP_TRUE : OP_FALSE) << ToByteVector(vch);
        return true;
      }

    case TX_LEASINGREWARD: {
        // 0. TRXHASH
        // 1. N
        keyID = CKeyID(uint160(vSolutions[2]));
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
        {
            LogPrintf("*** solver failed to sign leasing reward \n");
            return false;
        }
        else
        {
            CPubKey vch;
            if (!keystore.GetPubKey(keyID, vch))
                return error("%s : Unable to get public key from keyID for leasing reward" , __func__);
            scriptSigRet << ToByteVector(vch);
        }
        return true;
      }
    }

    LogPrintf("*** solver no case met \n");
    return false;
}

bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CMutableTransaction& txTo, unsigned int nIn, int nHashType, bool fColdStake, bool fLeasing, bool fForceLeaserSign)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];
   CScript subscript;
   std::vector<valtype> result;
   bool P2SH = false;
    // Leave out the signature from the hash, since a signature can't sign itself.
    // The checksig op will also drop the signatures from its hash.
    uint256 hash = SignatureHash(fromPubKey, txTo, nIn, nHashType);

    txnouttype whichType;
    if (!SignStep(keystore, fromPubKey, hash, nHashType, txin.scriptSig, whichType, result, fColdStake, fLeasing,
                  fForceLeaserSign))
        return false;

    if (whichType == TX_SCRIPTHASH)
    {
        // SignStep returns the subscript that need to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        subscript = txin.scriptSig;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript, txTo, nIn, nHashType);

        bool fSolved = SignStep(keystore, subscript, hash2, nHashType, txin.scriptSig, whichType, result)
                       && whichType != TX_SCRIPTHASH;

        // Append serialized subscript whether or not it is completely signed:
        P2SH = true;
        txin.scriptSig = subscript;
        if (!fSolved) return false;
    }

    if (whichType == TX_WITNESS_V0_KEYHASH)
    {
        //Temporary return false for any types of witness programs before full integration of bitcoin witness signature check
        return false;

        //CKeyID keyID(uint160(txin.scriptSig));

        CScript subscript2;
        subscript2 << OP_DUP << OP_HASH160 << ToByteVector(txin.scriptSig) << OP_EQUALVERIFY << OP_CHECKSIG;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript2, txTo, nIn, nHashType);

        txnouttype subType;
        bool fSolved = SignStep(keystore, subscript2, hash2, nHashType, txin.scriptSig, subType,result);

        // Append serialized subscript whether or not it is completely signed:
        //txin.scriptSig << static_cast<valtype>(subscript);
        txin.scriptSig = subscript2;

        if (!fSolved) return false;
    }
    else if (whichType == TX_WITNESS_V0_SCRIPTHASH)
    {
        //Temporary return false for any types of witness programs before full integration of bitcoin witness signature check
        return false;

        CScript subscript2 = txin.scriptSig;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript2, txTo, nIn, nHashType);

        txnouttype subType;
        bool fSolved = SignStep(keystore, subscript2, hash2, nHashType, txin.scriptSig, subType, result)
                       && subType != TX_SCRIPTHASH
                       && subType != TX_WITNESS_V0_SCRIPTHASH
                       && subType != TX_WITNESS_V0_KEYHASH;

        // Append serialized subscript whether or not it is completely signed:
        //txin.scriptSig << static_cast<valtype>(subscript);
        txin.scriptSig = subscript2;

        if (!fSolved) return false;

    } else if (whichType == TX_WITNESS_UNKNOWN) {
        // This is for the future soft forks of Bitcoin
        return false;
    }

    CScriptWitness witness;
    witness.stack.clear();

    if(P2SH)
    {
       result.push_back(std::vector<unsigned char>(subscript.begin(), subscript.end()));
       txin.scriptSig = PushAll(result);
    }
    // Test solution
    //return BTC::VerifyScript(txin.scriptSig, fromPubKey, &witness, STANDARD_SCRIPT_VERIFY_FLAGS, BTC::MutableTransactionSignatureChecker(&txTo, nIn));
    return VerifyScript(txin.scriptSig, fromPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, MutableTransactionSignatureChecker(&txTo, nIn));
}

bool SignSignature(const CKeyStore &keystore, const CTransaction& txFrom, CMutableTransaction& txTo, unsigned int nIn, int nHashType, bool fColdStake, bool fLeasing, bool fForceLeaserSign)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];
    assert(txin.prevout.n < txFrom.vout.size());
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    return SignSignature(keystore, txout.scriptPubKey, txTo, nIn, nHashType, fColdStake, fLeasing, fForceLeaserSign);
}

/*static CScript PushAll(const std::vector<valtype>& values)
{
    CScript result;
    for (const valtype& v : values)
        result << v;
    return result;
}*/

static CScript PushAll(const std::vector<valtype>& values)
{
   CScript result;
   for (const valtype& v : values) {
      if (v.size() == 0) {
         result << OP_0;
      } else if (v.size() == 1 && v[0] >= 1 && v[0] <= 16) {
         result << CScript::EncodeOP_N(v[0]);
      } else if (v.size() == 1 && v[0] == 0x81) {
         result << OP_1NEGATE;
      } else {
         result << v;
      }
   }
   return result;
}

static CScript CombineMultisig(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                               const std::vector<valtype>& vSolutions,
                               const std::vector<valtype>& sigs1, const std::vector<valtype>& sigs2)
{
    // Combine all the signatures we've got:
    std::set<valtype> allsigs;
    for (const valtype& v : sigs1)
    {
        if (!v.empty())
            allsigs.insert(v);
    }
    for (const valtype& v : sigs2)
    {
        if (!v.empty())
            allsigs.insert(v);
    }

    // Build a map of pubkey -> signature by matching sigs to pubkeys:
    assert(vSolutions.size() > 1);
    unsigned int nSigsRequired = vSolutions.front()[0];
    unsigned int nPubKeys = vSolutions.size()-2;
    std::map<valtype, valtype> sigs;
    for (const valtype& sig : allsigs)
    {
        for (unsigned int i = 0; i < nPubKeys; i++)
        {
            const valtype& pubkey = vSolutions[i+1];
            if (sigs.count(pubkey))
                continue; // Already got a sig for this pubkey

            if (TransactionSignatureChecker(&txTo, nIn).CheckSig(sig, pubkey, scriptPubKey))
            {
                sigs[pubkey] = sig;
                break;
            }
        }
    }
    // Now build a merged CScript:
    unsigned int nSigsHave = 0;
    CScript result; result << OP_0; // pop-one-too-many workaround
    for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++)
    {
        if (sigs.count(vSolutions[i+1]))
        {
            result << sigs[vSolutions[i+1]];
            ++nSigsHave;
        }
    }
    // Fill any missing with OP_0:
    for (unsigned int i = nSigsHave; i < nSigsRequired; i++)
        result << OP_0;

    return result;
}

static CScript CombineSignatures(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                                 const txnouttype txType, const std::vector<valtype>& vSolutions,
                                 std::vector<valtype>& sigs1, std::vector<valtype>& sigs2)
{
    switch (txType)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
    case TX_ZEROCOINMINT:
    case TX_WITNESS_UNKNOWN:
        // Don't know anything about this, assume bigger one is correct:
        if (sigs1.size() >= sigs2.size())
            return PushAll(sigs1);
        return PushAll(sigs2);
    case TX_PUBKEY:
    case TX_PUBKEYHASH:
    case TX_COLDSTAKE:
    case TX_LEASE_CLTV:
    case TX_LEASE:
    case TX_LEASINGREWARD:
    case TX_WITNESS_V0_SCRIPTHASH:
    case TX_WITNESS_V0_KEYHASH:
        // Signatures are bigger than placeholders or empty scripts:
        if (sigs1.empty() || sigs1[0].empty())
            return PushAll(sigs2);
        return PushAll(sigs1);
    case TX_SCRIPTHASH:
        if (sigs1.empty() || sigs1.back().empty())
            return PushAll(sigs2);
        else if (sigs2.empty() || sigs2.back().empty())
            return PushAll(sigs1);
        else
        {
            // Recur to combine:
            valtype spk = sigs1.back();
            CScript pubKey2(spk.begin(), spk.end());

            txnouttype txType2;
            std::vector<std::vector<unsigned char> > vSolutions2;
            Solver(pubKey2, txType2, vSolutions2);
            sigs1.pop_back();
            sigs2.pop_back();
            CScript result = CombineSignatures(pubKey2, txTo, nIn, txType2, vSolutions2, sigs1, sigs2);
            result << spk;
            return result;
        }
    case TX_MULTISIG:
        return CombineMultisig(scriptPubKey, txTo, nIn, vSolutions, sigs1, sigs2);
    }

    return CScript();
}

CScript CombineSignatures(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                          const CScript& scriptSig1, const CScript& scriptSig2)
{
    txnouttype txType;
    std::vector<std::vector<unsigned char> > vSolutions;
    Solver(scriptPubKey, txType, vSolutions);

    std::vector<valtype> stack1;
    EvalScript(stack1, scriptSig1, SCRIPT_VERIFY_STRICTENC, BaseSignatureChecker());
    std::vector<valtype> stack2;
    EvalScript(stack2, scriptSig2, SCRIPT_VERIFY_STRICTENC, BaseSignatureChecker());

    return CombineSignatures(scriptPubKey, txTo, nIn, txType, vSolutions, stack1, stack2);
}

namespace BTC {
   namespace
   {

      static CScript PushAll(const std::vector<valtype>& values)
      {
         CScript result;
         for (const valtype& v : values) {
            if (v.size() == 0) {
               result << OP_0;
            } else if (v.size() == 1 && v[0] >= 1 && v[0] <= 16) {
               result << CScript::EncodeOP_N(v[0]);
            } else {
               result << v;
            }
         }
         return result;
      }

      struct Stacks
      {
         std::vector<valtype> script;
         std::vector<valtype> witness;

         Stacks() {}
         explicit Stacks(const std::vector<valtype>& scriptSigStack_) : script(scriptSigStack_), witness() {}
         explicit Stacks(const SignatureData& data) : witness(data.scriptWitness.stack) {
            BTC::EvalScript(script, data.scriptSig, SCRIPT_VERIFY_STRICTENC, BTC::BaseSignatureChecker(), SigVersion::BASE);
         }

         SignatureData Output() const {
            SignatureData result;
            result.scriptSig = BTC::PushAll(script);
            result.scriptWitness.stack = witness;
            return result;
         }
      };
   }



   TransactionSignatureCreator::TransactionSignatureCreator(const CKeyStore* keystoreIn, const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, int nHashTypeIn) : BaseSignatureCreator(keystoreIn), txTo(txToIn), nIn(nInIn), nHashType(nHashTypeIn), amount(amountIn), checker(txTo, nIn, amountIn) {}

   bool TransactionSignatureCreator::CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& address, const CScript& scriptCode, SigVersion sigversion) const
   {
      CKey key;
      if (!keystore->GetKey(address, key))
         return false;

      // Signing with uncompressed keys is disabled in witness scripts
      if (sigversion == SigVersion::WITNESS_V0 && !key.IsCompressed())
         return false;

      uint256 hash = BTC::SignatureHash(scriptCode, *txTo, nIn, nHashType, amount, sigversion);
      if (!key.Sign(hash, vchSig))
         return false;
      vchSig.push_back((unsigned char)nHashType);
      return true;
   }

   static bool Sign1(const CKeyID& address, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret, SigVersion sigversion)
   {
      std::vector<unsigned char> vchSig;
      if (!creator.CreateSig(vchSig, address, scriptCode, sigversion))
         return false;
      ret.push_back(vchSig);
      return true;
   }

   static bool SignN(const std::vector<valtype>& multisigdata, const BaseSignatureCreator& creator, const CScript& scriptCode, std::vector<valtype>& ret, SigVersion sigversion)
   {
      int nSigned = 0;
      int nRequired = multisigdata.front()[0];
      for (unsigned int i = 1; i < multisigdata.size()-1 && nSigned < nRequired; i++)
      {
         const valtype& pubkey = multisigdata[i];
         CKeyID keyID = CPubKey(pubkey).GetID();
         if (Sign1(keyID, creator, scriptCode, ret, sigversion))
            ++nSigned;
      }
      return nSigned==nRequired;
   }


   static std::vector<valtype> CombineMultisig(const CScript& scriptPubKey, const BTC::BaseSignatureChecker& checker,
                                               const std::vector<valtype>& vSolutions,
                                               const std::vector<valtype>& sigs1, const std::vector<valtype>& sigs2, SigVersion sigversion)
   {
      // Combine all the signatures we've got:
      std::set<valtype> allsigs;
      for (const valtype& v : sigs1)
      {
         if (!v.empty())
            allsigs.insert(v);
      }
      for (const valtype& v : sigs2)
      {
         if (!v.empty())
            allsigs.insert(v);
      }

      // Build a map of pubkey -> signature by matching sigs to pubkeys:
      assert(vSolutions.size() > 1);
      unsigned int nSigsRequired = vSolutions.front()[0];
      unsigned int nPubKeys = vSolutions.size()-2;
      std::map<valtype, valtype> sigs;
      for (const valtype& sig : allsigs)
      {
         for (unsigned int i = 0; i < nPubKeys; i++)
         {
            const valtype& pubkey = vSolutions[i+1];
            if (sigs.count(pubkey))
               continue; // Already got a sig for this pubkey

            if (checker.CheckSig(sig, pubkey, scriptPubKey, sigversion))
            {
               sigs[pubkey] = sig;
               break;
            }
         }
      }
      // Now build a merged CScript:
      unsigned int nSigsHave = 0;
      std::vector<valtype> result; result.push_back(valtype()); // pop-one-too-many workaround
      for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++)
      {
         if (sigs.count(vSolutions[i+1]))
         {
            result.push_back(sigs[vSolutions[i+1]]);
            ++nSigsHave;
         }
      }
      // Fill any missing with OP_0:
      for (unsigned int i = nSigsHave; i < nSigsRequired; i++)
         result.push_back(valtype());

      return result;
   }

   static Stacks CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                                   const txnouttype txType, const std::vector<valtype>& vSolutions,
                                   Stacks sigs1, Stacks sigs2, SigVersion sigversion)
   {
      switch (txType)
      {
         case TX_NONSTANDARD:
         case TX_NULL_DATA:
         case TX_WITNESS_UNKNOWN:
            // Don't know anything about this, assume bigger one is correct:
            if (sigs1.script.size() >= sigs2.script.size())
               return sigs1;
            return sigs2;
         case TX_PUBKEY:
         case TX_PUBKEYHASH:
         case TX_COLDSTAKE:
         case TX_LEASE_CLTV:
         case TX_LEASE:
         case TX_LEASINGREWARD:
            // Signatures are bigger than placeholders or empty scripts:
            if (sigs1.script.empty() || sigs1.script[0].empty())
               return sigs2;
            return sigs1;
         case TX_WITNESS_V0_KEYHASH:
            // Signatures are bigger than placeholders or empty scripts:
            if (sigs1.witness.empty() || sigs1.witness[0].empty())
               return sigs2;
            return sigs1;
         case TX_SCRIPTHASH:
            if (sigs1.script.empty() || sigs1.script.back().empty())
               return sigs2;
            else if (sigs2.script.empty() || sigs2.script.back().empty())
               return sigs1;
            else
            {
               // Recur to combine:
               valtype spk = sigs1.script.back();
               CScript pubKey2(spk.begin(), spk.end());

               txnouttype txType2;
               std::vector<std::vector<unsigned char> > vSolutions2;
               Solver(pubKey2, txType2, vSolutions2);
               sigs1.script.pop_back();
               sigs2.script.pop_back();
               Stacks result = CombineSignatures(pubKey2, checker, txType2, vSolutions2, sigs1, sigs2, sigversion);
               result.script.push_back(spk);
               return result;
            }
         case TX_MULTISIG:
            return Stacks(CombineMultisig(scriptPubKey, checker, vSolutions, sigs1.script, sigs2.script, sigversion));
         case TX_WITNESS_V0_SCRIPTHASH:
            if (sigs1.witness.empty() || sigs1.witness.back().empty())
               return sigs2;
            else if (sigs2.witness.empty() || sigs2.witness.back().empty())
               return sigs1;
            else
            {
               // Recur to combine:
               CScript pubKey2(sigs1.witness.back().begin(), sigs1.witness.back().end());
               txnouttype txType2;
               std::vector<valtype> vSolutions2;
               Solver(pubKey2, txType2, vSolutions2);
               sigs1.witness.pop_back();
               sigs1.script = sigs1.witness;
               sigs1.witness.clear();
               sigs2.witness.pop_back();
               sigs2.script = sigs2.witness;
               sigs2.witness.clear();
               Stacks result = CombineSignatures(pubKey2, checker, txType2, vSolutions2, sigs1, sigs2, SigVersion::WITNESS_V0);
               result.witness = result.script;
               result.script.clear();
               result.witness.push_back(valtype(pubKey2.begin(), pubKey2.end()));
               return result;
            }
         default:
            return Stacks();
      }
   }

   SignatureData CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker,
                                   const SignatureData& scriptSig1, const SignatureData& scriptSig2)
   {
      txnouttype txType;
      std::vector<std::vector<unsigned char> > vSolutions;
      Solver(scriptPubKey, txType, vSolutions);

      return CombineSignatures(scriptPubKey, checker, txType, vSolutions, Stacks(scriptSig1), Stacks(scriptSig2), SigVersion::BASE).Output();
   }

   void UpdateTransaction(CMutableTransaction& tx, unsigned int nIn, const SignatureData& data)
   {
      assert(tx.vin.size() > nIn);
      tx.vin[nIn].scriptSig = data.scriptSig;
      tx.vin[nIn].scriptWitness = data.scriptWitness;
   }

   SignatureData DataFromTransaction(const CMutableTransaction& tx, unsigned int nIn)
   {
      SignatureData data;
      assert(tx.vin.size() > nIn);
      data.scriptSig = tx.vin[nIn].scriptSig;
      data.scriptWitness = tx.vin[nIn].scriptWitness;
      return data;
   }

   /**
 * Sign scriptPubKey using signature made with creator.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey can't be signed),
 * unless whichTypeRet is TxoutType::SCRIPTHASH, in which case scriptSigRet is the redemption script.
 * Returns false if scriptPubKey could not be completely satisfied.
 */
   static bool SignStep(const BaseSignatureCreator& creator, const CScript& scriptPubKey,
                        std::vector<valtype>& ret, txnouttype& whichTypeRet, SigVersion sigversion, bool fColdStake = false,
                        bool fLeasing = false, bool fForceLeaserSign = false)
   {
      CScript scriptRet;
      uint160 h160;
      ret.clear();

      std::vector<valtype> vSolutions;
      if (!Solver(scriptPubKey, whichTypeRet, vSolutions))
         return false;

      CKeyID keyID;
      switch (whichTypeRet)
      {
         case TX_NONSTANDARD:
         case TX_NULL_DATA:
         case TX_WITNESS_UNKNOWN:
         case TX_ZEROCOINMINT:
            return false;
         case TX_PUBKEY:
            keyID = CPubKey(vSolutions[0]).GetID();
            return BTC::Sign1(keyID, creator, scriptPubKey, ret, sigversion);
         case TX_PUBKEYHASH:
            keyID = CKeyID(uint160(vSolutions[0]));
            if (!BTC::Sign1(keyID, creator, scriptPubKey, ret, sigversion))
               return false;
            else
            {
               CPubKey vch;
               creator.KeyStore().GetPubKey(keyID, vch);
               ret.push_back(ToByteVector(vch));
            }
            return true;
         case TX_SCRIPTHASH:
            if (creator.KeyStore().GetCScript(uint160(vSolutions[0]), scriptRet)) {
               ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
               return true;
            }
            return false;

         case TX_MULTISIG:
            ret.push_back(valtype()); // workaround CHECKMULTISIG bug
            return (BTC::SignN(vSolutions, creator, scriptPubKey, ret, sigversion));

         case TX_WITNESS_V0_KEYHASH:
            ret.push_back(vSolutions[0]);
            return true;

         case TX_WITNESS_V0_SCRIPTHASH:
            CRIPEMD160().Write(&vSolutions[0][0], vSolutions[0].size()).Finalize(h160.begin());
            if (creator.KeyStore().GetCScript(h160, scriptRet)) {
               ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
               return true;
            }
            return false;

         case TX_COLDSTAKE: {
            if (fColdStake) {
               // sign with the cold staker key
               keyID = CKeyID(uint160(vSolutions[0]));
            } else {
               // sign with the owner key
               keyID = CKeyID(uint160(vSolutions[1]));
            }
            //if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
            if(!BTC::Sign1(keyID, creator, scriptPubKey, ret, sigversion))
               return error("*** %s: failed to sign with the %s key.",
                            __func__, fColdStake ? "cold staker" : "owner");
            CPubKey vch;
            if (!creator.KeyStore().GetPubKey(keyID, vch))
               return error("%s : Unable to get public key from keyID", __func__);
            scriptRet << ret[0];
            scriptRet << (fColdStake ? (int)OP_TRUE : OP_FALSE) << ToByteVector(vch);
            ret.clear();
            ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
            return true;
         }

         case TX_LEASE_CLTV:
         case TX_LEASE: {
            if (fLeasing) {
               if (!fForceLeaserSign) {
                  LogPrintf("*** solver met leaser, who can't sign \n");
                  return false;
               }
               // sign with the leaser key
               keyID = (whichTypeRet == TX_LEASE) ? CKeyID(uint160(vSolutions[0])): CKeyID(uint160(vSolutions[1]));
            } else {
               // sign with the owner key
               keyID = (whichTypeRet == TX_LEASE) ? CKeyID(uint160(vSolutions[1])): CKeyID(uint160(vSolutions[2]));
            }
            //if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet, ret))
            if(!BTC::Sign1(keyID, creator, scriptPubKey, ret, sigversion))
               return error("*** %s: failed to sign with the %s key.",
                            __func__, fLeasing ? "leaser" : "owner");
            CPubKey vch;
            if (!creator.KeyStore().GetPubKey(keyID, vch))
               return error("%s : Unable to get public key from keyID", __func__);

            scriptRet << ret[0];
            scriptRet << (fLeasing ? (int)OP_TRUE : OP_FALSE) << ToByteVector(vch);
            ret.clear();
            ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
            return true;
         }

         case TX_LEASINGREWARD: {
            // 0. TRXHASH
            // 1. N
            keyID = CKeyID(uint160(vSolutions[2]));
            //if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet,ret))
            if(!BTC::Sign1(keyID, creator, scriptPubKey, ret, sigversion))
            {
               LogPrintf("*** solver failed to sign leasing reward \n");
               return false;
            }
            else
            {
               CPubKey vch;
               if (!creator.KeyStore().GetPubKey(keyID, vch))
                  return error("%s : Unable to get public key from keyID for leasing reward" , __func__);
               ret.push_back(ToByteVector(vch));
            }
            return true;
         }

         default:
            return false;
      }
   }

   bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& fromPubKey, SignatureData& sigdata, bool fColdStake, bool fLeasing, bool fForceLeaserSign)
   {
      CScript script = fromPubKey;
      std::vector<valtype> result;
      txnouttype whichType;
      bool solved = BTC::SignStep(creator, script, result, whichType, SigVersion::BASE, fColdStake, fLeasing, fForceLeaserSign);
      bool P2SH = false;
      CScript subscript;
      sigdata.scriptWitness.stack.clear();
      int flags = STANDARD_SCRIPT_VERIFY_FLAGS;

      if (solved && whichType == TX_SCRIPTHASH)
      {
         // Solver returns the subscript that needs to be evaluated;
         // the final scriptSig is the signatures from that
         // and then the serialized subscript:
         script = subscript = CScript(result[0].begin(), result[0].end());
         solved = solved && BTC::SignStep(creator, script, result, whichType, SigVersion::BASE) && whichType != TX_SCRIPTHASH;
         P2SH = true;
      }

      if (solved && whichType == TX_WITNESS_V0_KEYHASH)
      {
         CScript witnessscript;
         witnessscript << OP_DUP << OP_HASH160 << ToByteVector(result[0]) << OP_EQUALVERIFY << OP_CHECKSIG;
         txnouttype subType;
         solved = solved && BTC::SignStep(creator, witnessscript, result, subType, SigVersion::WITNESS_V0);
         sigdata.scriptWitness.stack = result;
         result.clear();
      }
      else if (solved && whichType == TX_WITNESS_V0_SCRIPTHASH)
      {
         CScript witnessscript(result[0].begin(), result[0].end());
         txnouttype subType;
         solved = solved && BTC::SignStep(creator, witnessscript, result, subType, SigVersion::WITNESS_V0) && subType != TX_SCRIPTHASH && subType != TX_WITNESS_V0_SCRIPTHASH && subType != TX_WITNESS_V0_KEYHASH;
         result.push_back(std::vector<unsigned char>(witnessscript.begin(), witnessscript.end()));
         sigdata.scriptWitness.stack = result;
         result.clear();
      }

      if (P2SH) {
         result.push_back(std::vector<unsigned char>(subscript.begin(), subscript.end()));
      }
      if(solved && (whichType == TX_LEASE_CLTV || whichType == TX_LEASE || whichType == TX_COLDSTAKE))
         sigdata.scriptSig = CScript(result[0].begin(), result[0].end());
      else
         sigdata.scriptSig = PushAll(result);

      //allow checklocktime for leasing clvt trx
      if(whichType == TX_LEASE_CLTV)
         flags |= SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;

      // Test solution
      return solved && BTC::VerifyScript(sigdata.scriptSig, fromPubKey, &sigdata.scriptWitness, flags, creator.Checker());
   }


   namespace {
/** Dummy signature checker which accepts all signatures. */
      class DummySignatureChecker : public BaseSignatureChecker
      {
      public:
         DummySignatureChecker() {}

         bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const override
         {
            return true;
         }
      };
      const DummySignatureChecker dummyChecker;
   } // namespace

   const BaseSignatureChecker& DummySignatureCreator::Checker() const
   {
      return dummyChecker;
   }

   bool DummySignatureCreator::CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode, SigVersion sigversion) const
   {
      // Create a dummy signature that is a valid DER-encoding
      vchSig.assign(72, '\000');
      vchSig[0] = 0x30;
      vchSig[1] = 69;
      vchSig[2] = 0x02;
      vchSig[3] = 33;
      vchSig[4] = 0x01;
      vchSig[4 + 33] = 0x02;
      vchSig[5 + 33] = 32;
      vchSig[6 + 33] = 0x01;
      vchSig[6 + 33 + 32] = SIGHASH_ALL;
      return true;
   }

   bool IsSolvable(const CKeyStore& store, const CScript& script)
   {
      // This check is to make sure that the script we created can actually be solved for and signed by us
      // if we were to have the private keys. This is just to make sure that the script is valid and that,
      // if found in a transaction, we would still accept and relay that transaction. In particular,
      // it will reject witness outputs that require signing with an uncompressed public key.
      DummySignatureCreator creator(&store);
      SignatureData sigs;
      // Make sure that STANDARD_SCRIPT_VERIFY_FLAGS includes SCRIPT_VERIFY_WITNESS_PUBKEYTYPE, the most
      // important property this function is designed to test for.
      static_assert(STANDARD_SCRIPT_VERIFY_FLAGS & SCRIPT_VERIFY_WITNESS_PUBKEYTYPE, "IsSolvable requires standard script flags to include WITNESS_PUBKEYTYPE");
      if (ProduceSignature(creator, script, sigs)) {
         // VerifyScript check is just defensive, and should never fail.
         assert(VerifyScript(sigs.scriptSig, script, &sigs.scriptWitness, STANDARD_SCRIPT_VERIFY_FLAGS, creator.Checker()));
         return true;
      }
      return false;
   }
}


