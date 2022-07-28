// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SCRIPT_STANDARD_H
#define BITCOIN_SCRIPT_STANDARD_H

#include "script/interpreter.h"
#include "uint256.h"
#include <util/hash_type.h>

#include <boost/variant.hpp>

#include <stdint.h>
#include <variant>

//contract executions with less gas than this are not standard
//Make sure is always equal or greater than MINIMUM_GAS_LIMIT (which we can't reference here due to insane header dependency chains)
static const uint64_t STANDARD_MINIMUM_GAS_LIMIT = 10000;
//contract executions with a price cheaper than this (in satoshis) are not standard
//TODO this needs to be controlled by DGP and needs to be propogated from consensus parameters
static const uint64_t STANDARD_MINIMUM_GAS_PRICE = 1;


class CKeyID;
class CScript;
struct ScriptHash;

/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public BaseHash<uint160>
{
public:
   CScriptID() : BaseHash() {}
   explicit CScriptID(const CScript& in);
   explicit CScriptID(const uint160& in) : BaseHash(in) {}
   explicit CScriptID(const ScriptHash& in);
};

static const unsigned int MAX_OP_RETURN_RELAY = 83;      //!< bytes (+1 for OP_RETURN, +2 for the pushdata opcodes)
extern unsigned nMaxDatacarrierBytes;

/**
 * Mandatory script verification flags that all new blocks must comply with for
 * them to be valid. (but old blocks may not comply with) Currently just P2SH,
 * but in the future other flags may be added, such as a soft-fork to enforce
 * strict DER encoding.
 *
 * Failing one of these tests may trigger a DoS ban - see CheckInputs() for
 * details.
 */
static const unsigned int MANDATORY_SCRIPT_VERIFY_FLAGS = SCRIPT_VERIFY_P2SH;


/**
 * Standard script verification flags that standard transactions will comply
 * with. However scripts violating these flags may still be present in valid
 * blocks and we must accept those blocks.
 */
static constexpr unsigned int STANDARD_SCRIPT_VERIFY_FLAGS_N = MANDATORY_SCRIPT_VERIFY_FLAGS |
                                                             SCRIPT_VERIFY_DERSIG |
                                                             SCRIPT_VERIFY_STRICTENC |
                                                             SCRIPT_VERIFY_MINIMALDATA |
                                                             SCRIPT_VERIFY_NULLDUMMY |
                                                             SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS |
                                                             SCRIPT_VERIFY_CLEANSTACK |
                                                             SCRIPT_VERIFY_MINIMALIF |
                                                             SCRIPT_VERIFY_NULLFAIL |
                                                             SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY |
                                                             SCRIPT_VERIFY_CHECKSEQUENCEVERIFY |
                                                             SCRIPT_VERIFY_LOW_S |
                                                             SCRIPT_VERIFY_WITNESS |
                                                             SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM |
                                                             SCRIPT_VERIFY_WITNESS_PUBKEYTYPE |
                                                             SCRIPT_VERIFY_CONST_SCRIPTCODE;


enum txnouttype
{
    TX_NONSTANDARD,
    // 'standard' transaction types:
    TX_PUBKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIG,
    TX_NULL_DATA,
    TX_ZEROCOINMINT,
    TX_COLDSTAKE,
    TX_CREATE_SENDER,
    TX_CALL_SENDER,
    TX_CREATE,
    TX_CALL,
    TX_WITNESS_UNKNOWN,
    TX_WITNESS_V0_SCRIPTHASH,
    TX_WITNESS_V0_KEYHASH,
    TX_WITNESS_V1_TAPROOT,
    TX_LEASE,
    TX_LEASINGREWARD,
    TX_LEASE_CLTV
};

class CNoDestination {
public:
    friend bool operator==(const CNoDestination &a, const CNoDestination &b) { return true; }
    friend bool operator<(const CNoDestination &a, const CNoDestination &b) { return true; }
};

struct PKHash : public BaseHash<uint160>
{
   PKHash() : BaseHash() {}
   explicit PKHash(const uint160& hash) : BaseHash(hash) {}
   explicit PKHash(const CPubKey& pubkey);
   explicit PKHash(const CKeyID& pubkey_id);
};
CKeyID ToKeyID(const PKHash& key_hash);

struct WitnessV0KeyHash;
struct ScriptHash : public BaseHash<uint160>
{
   ScriptHash() : BaseHash() {}
   // These don't do what you'd expect.
   // Use ScriptHash(GetScriptForDestination(...)) instead.
   explicit ScriptHash(const WitnessV0KeyHash& hash) = delete;
   explicit ScriptHash(const PKHash& hash) = delete;

   explicit ScriptHash(const uint160& hash) : BaseHash(hash) {}
   explicit ScriptHash(const CScript& script);
   explicit ScriptHash(const CScriptID& script);
};

struct WitnessV0ScriptHash : public BaseHash<uint256>
{
   WitnessV0ScriptHash() : BaseHash() {}
   explicit WitnessV0ScriptHash(const uint256& hash) : BaseHash(hash) {}
   explicit WitnessV0ScriptHash(const CScript& script);
};

struct WitnessV0KeyHash : public BaseHash<uint160>
{
   WitnessV0KeyHash() : BaseHash() {}
   explicit WitnessV0KeyHash(const uint160& hash) : BaseHash(hash) {}
   explicit WitnessV0KeyHash(const CPubKey& pubkey);
   explicit WitnessV0KeyHash(const PKHash& pubkey_hash);
};
CKeyID ToKeyID(const WitnessV0KeyHash& key_hash);

struct WitnessV1Taproot : public XOnlyPubKey
{
   WitnessV1Taproot() : XOnlyPubKey() {}
   explicit WitnessV1Taproot(const XOnlyPubKey& xpk) : XOnlyPubKey(xpk) {}
};

//! CTxDestination subtype to encode any future Witness version
struct WitnessUnknown
{
    unsigned int version;
    unsigned int length;
    unsigned char program[40];

    friend bool operator==(const WitnessUnknown& w1, const WitnessUnknown& w2) {
        if (w1.version != w2.version) return false;
        if (w1.length != w2.length) return false;
        return std::equal(w1.program, w1.program + w1.length, w2.program);
    }

    friend bool operator<(const WitnessUnknown& w1, const WitnessUnknown& w2) {
        if (w1.version < w2.version) return true;
        if (w1.version > w2.version) return false;
        if (w1.length < w2.length) return true;
        if (w1.length > w2.length) return false;
        return std::lexicographical_compare(w1.program, w1.program + w1.length, w2.program, w2.program + w2.length);
    }
};
/**
 * A txout script template with a specific destination. It is either:
 *  * CNoDestination: no destination set
 *  * CKeyID: TX_PUBKEYHASH destination
 *  * CScriptID: TX_SCRIPTHASH destination
 */
using CTxDestination = std::variant<CNoDestination, PKHash, ScriptHash, WitnessV0ScriptHash, WitnessV0KeyHash, WitnessV1Taproot, WitnessUnknown>;

enum addresstype
{
   PUBKEYHASH = 1,
   SCRIPTHASH = 2,
   WITNESSSCRIPTHASH = 3,
   WITNESSPUBKEYHASH = 4,
   NONSTANDARD = 5
};

///////////////////////////////////////////qtum
/** Parse a output public key for the sender public key and sender signature. */
bool ExtractSenderData(const CScript& outputPubKey, CScript* senderPubKey, CScript* senderSig);
bool GetSenderPubKey(const CScript& outputPubKey, CScript& senderPubKey);

const char* GetTxnOutputType(txnouttype t);

bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet, bool contractConsensus=false, bool allowEmptySenderSig=false);
int ScriptSigArgsExpected(txnouttype t, const std::vector<std::vector<unsigned char> >& vSolutions);
bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType);
bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet, bool fColdStake = false, bool fLease = false);
bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet, txnouttype *typeRet);
bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<CTxDestination>& addressRet, int& nRequiredRet);
/** Check whether a CTxDestination is a CNoDestination. */
bool IsValidDestination(const CTxDestination& dest);

/** Check whether a CTxDestination can be used as contract sender address. */
bool IsValidContractSenderAddress(const CTxDestination& dest);

CScript GetScriptForDestination(const CTxDestination& dest);
/** Generate a P2PK script for the given pubkey. */
CScript GetScriptForRawPubKey(const CPubKey& pubkey);
/** Generate a multisig script. */
CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys);
CScript GetScriptForStakeDelegation(const CKeyID& stakingKey, const CKeyID& spendingKey);
CScript GetScriptForLeasing(const CKeyID& leaserKey, const CKeyID& ownerKey);
CScript GetScriptForLeasingCLTV(const CKeyID& leaserKey, const CKeyID& ownerKey, uint32_t nLockTime);
CScript GetScriptForLeasingReward(const COutPoint& outPoint, const CTxDestination& dest);

/**
 * Generate a pay-to-witness script for the given redeem script. If the redeem
 * script is P2PK or P2PKH, this returns a P2WPKH script, otherwise it returns a
 * P2WSH script.
 *
 * TODO: replace calls to GetScriptForWitness with GetScriptForDestination using
 * the various witness-specific CTxDestination subtypes.
 */
CScript GetScriptForWitness(const CScript& redeemscript);

struct ShortestVectorFirstComparator
{
   bool operator()(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) const
   {
      if (a.size() < b.size()) return true;
      if (a.size() > b.size()) return false;
      return a < b;
   }
};

struct TaprootSpendData
{
   /** The BIP341 internal key. */
   XOnlyPubKey internal_key;
   /** The Merkle root of the script tree (0 if no scripts). */
   uint256 merkle_root;
   /** Map from (script, leaf_version) to (sets of) control blocks.
    *  More than one control block for a given script is only possible if it
    *  appears in multiple branches of the tree. We keep them all so that
    *  inference can reconstruct the full tree. Within each set, the control
    *  blocks are sorted by size, so that the signing logic can easily
    *  prefer the cheapest one. */
   std::map<std::pair<CScript, int>, std::set<std::vector<unsigned char>, ShortestVectorFirstComparator>> scripts;
   /** Merge other TaprootSpendData (for the same scriptPubKey) into this. */
   void Merge(TaprootSpendData other);
};

#endif // BITCOIN_SCRIPT_STANDARD_H
