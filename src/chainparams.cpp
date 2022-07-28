// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include "util/convert.h"
#include <libdevcore/SHA3.h>
#include <libdevcore/RLP.h>
#include <assert.h>

#include <boost/assign/list_of.hpp>
#include <limits>
#include <regex>

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"
#include "libethashseal/GenesisInfo.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (  0, uint256("0x"));

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    //1578332625, // * UNIX timestamp of last checkpoint block
    //5116987,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    //2000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (  0, uint256("0x"));

    static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    //1575145155,
    //2971390,
    //250
    };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

libzerocoin::ZerocoinParams* CChainParams::Zerocoin_Params(bool useModulusV1) const
{
    assert(this);
    static CBigNum bnHexModulus = 0;
    if (!bnHexModulus)
        bnHexModulus.SetHex(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsHex = libzerocoin::ZerocoinParams(bnHexModulus);
    static CBigNum bnDecModulus = 0;
    if (!bnDecModulus)
        bnDecModulus.SetDec(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsDec = libzerocoin::ZerocoinParams(bnDecModulus);

    if (useModulusV1)
        return &ZCParamsHex;

    return &ZCParamsDec;
}

bool CChainParams::HasStakeMinAgeOrDepth(const int contextHeight, const uint32_t contextTime,
        const int utxoFromBlockHeight, const uint32_t utxoFromBlockTime) const
{
    // BTCU from Genesis state
    if (utxoFromBlockHeight == 0)
        return true;

    // before stake modifier V2, the age required was 60 * 60 (1 hour).
    if (!IsStakeModifierV2(contextHeight))
        return (utxoFromBlockTime + nStakeMinAge <= contextTime);

    // after stake modifier V2, we require the utxo to be nStakeMinDepth deep in the chain
    return (contextHeight - utxoFromBlockHeight >= nStakeMinDepth);
}

int CChainParams::FutureBlockTimeDrift(const int nHeight) const
{
    if (IsTimeProtocolV2(nHeight))
        // PoS (TimeV2): 14 seconds
        return TimeSlotLength() - 1;

    // PoS (TimeV1): 3 minutes
    // PoW: 2 hours
    return (nHeight > LAST_POW_BLOCK()) ? nFutureTimeDriftPoS : nFutureTimeDriftPoW;
}

bool CChainParams::IsValidBlockTimeStamp(const int64_t nTime, const int nHeight) const
{
    // Before time protocol V2, blocks can have arbitrary timestamps
    if (!IsTimeProtocolV2(nHeight))
        return true;

    // Time protocol v2 requires time in slots
    return (nTime % TimeSlotLength()) == 0;
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x25;//0x90;
        pchMessageStart[1] = 0x7a;//0xc4;
        pchMessageStart[2] = 0xf3;//0xfd;
        pchMessageStart[3] = 0x11;//0xe9;
        vAlertPubKey = ParseHex("047e7293daa8765739db0bcd71ba5a74a6bbdd3440d1ca6b0fc49332292a21e0daeaad1184e7be80c641d159a5281a53e4c61f46619914743c582e51c42ea898d8");
        nDefaultPort = 3666;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // BTCU starting difficulty is 1 / 2^12
        bnProofOfStakeLimit = ~uint256(0) >> 24;
        bnProofOfStakeLimit_V2 = ~uint256(0) >> 20; // 60/4 = 15 ==> use 2**4 higher limit
        nSubsidyHalvingInterval = 210000;
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 8100; // 75%
        nRejectBlockOutdatedMajority = 10260; // 95%
        nToCheckBlockUpgradeMajority = 10800; // Approximate expected amount of blocks in 7 days (1440*7.5)
        nMinerThreads = 0;
        nTargetSpacing = 1 * 60;                        // 1 minute
        nTargetTimespan = 40 * 60;                      // 40 minutes
        nTimeSlotLength = 15;                           // 15 seconds
        nTargetTimespan_V2 = 2 * nTimeSlotLength * 60;  // 30 minutes
        nMaturity = 9;//for activating POS, there are 10 utxo's in the bitcoin chainstate, one for each validator.
        nStakeMinAge = 60 * 60;                         // 1 hour
        nStakeMinDepth = 9; //same as nMaturity
        nFutureTimeDriftPoW = 7200;
        nFutureTimeDriftPoS = 180;
        nMasternodeCountDrift = 20;
        nMinColdStakingAmount = 1 * COIN;
        nMinLeasingAmount = 1 * COIN;

        /** Height or Time Based Activations **/
        nLastPOWBlock = 0;
        nBtcuBadBlockTime = 0; // Skip nBit validation of Block 259201 per PR #915
        nBtcuBadBlocknBits = 0; // Skip nBit validation of Block 259201 per PR #915
        nModifierUpdateBlock = 0;
        nZerocoinStartHeight = 1;
        nZerocoinStartTime = 1583491266 - 1;
        nBlockEnforceSerialRange = 0; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 1; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 0; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 0; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 0; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 268200*COIN; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = 0; //!> The block that zerocoin v2 becomes active - roughly Tuesday, May 8, 2018 4:00:00 AM GMT
        nBlockDoubleAccumulated = 0;
        nEnforceNewSporkKey = 1583491266; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
        nRejectOldSporkKey = 1583491266 - 1; //!> Fully reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
        nBlockStakeModifierlV2 = nLastPOWBlock + 1;//1967000;
        nBIP65ActivationHeight = 0;
        // Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
        nBlockTimeProtocolV2 = nLastPOWBlock + 1;

        // Public coin spend enforcement
        nPublicZCSpends = 0;

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

        // Blocks v7
        nBlockLastAccumulatorCheckpoint = 0;
        nBlockV7StartHeight = nBlockTimeProtocolV2;

        // Leasing
        nLeasingRewardMaturity = 30; // 30 blocks
        nLeasingRewardPeriod = 7 * 24 * 60; // 1 weak
        nMaxLeasingRewards = 100;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = 0;
        nSupplyBeforeFakeSerial = 0;// zerocoin supply at block nFakeSerialBlockheightEnd

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "BBC News March 06 2020 Brexit preparations cost UK more than £4bn";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 250 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;


        std::vector<std::string> validatorsPubkeys = {
         "02e5b68ebea89e38340362c21adbbcc086485decbfe160b657f4357413058761f4",
         "02222a0a658afaed8ee0867a31a15100c24f0a5d106fde23776a376415d026c850",
         "0372697e2fdaab9fd8c884c8eacb0f14dbd7ebf08dccb7529f578c92c4bd86c2e5",
         "02b0cf66b4553eb8b35843bc95f6d15a2a37c9e6c8e2d88352064139f1d544b2c4",
         "037c932e25fd0ae81698cc7d6bef3f35fee65e46f8062731961137ef6d93b3491d",
         "03d079948de5fe9c189a3664c4aa74969ec4a24d5c670099eca7a79cabf8357641",
         "02cfadf65422ddf50162bec899bbe8f1cf8d839d8e74aad1064fb1e77408294a23",
         "0314e324c9df4023ba0377d923db4bbfbc344e0c00e63fe95488c74b5cf2243355",
         "03eba5c0f39462a42679f656ba57d8b877cf523c4d24636fb04f5e380b2edf92d4",
         "020270e971b840ddb5ccfa03728a7a71c3dd9b3e2629c5064cded9374c4c0cbc3a"
         };


        for(int i = 0; i < validatorsPubkeys.size(); i++)
        {
            CTxIn validatorVin(uint256(0), i);
            CPubKey validatorPubKey(ParseHex(validatorsPubkeys[i].data()));
        
            CValidatorRegister validatorReg(validatorVin, validatorPubKey);
            validatorReg.nTime = 0;
            txNew.validatorRegister.push_back(validatorReg);
        }
        
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
        genesis.nVersion = 8;
        genesis.nTime = 1583491266;
        genesis.nBits = 0x1e0ffff0;
        genesis.nNonce = 7860220;
        genesis.hashChainstate = uint256("0x789ee471178b992f2b979ab75726b0999703d1e3c0a0e21ae25969f2b92c5549");
        genesis.hashStateRoot = uint256(h256Touint(dev::h256("e965ffd002cd6ad0e2dc402b8044de833e06b23127ea8c3d80aec91410771495"))); // qtum
        genesis.hashUTXORoot = uint256(h256Touint(dev::sha3(dev::rlp("")))); // qtum

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x000004b9b0ce0013c3718e2a691097a462e388161ec8356463ad14d9e68077e0"));
        assert(genesis.hashMerkleRoot == uint256("0x42f64a5c362004b2fbbd763fa0691b1efa4319962081d7f030d223842b74794c"));
        
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1,66);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "bc";

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        nBudgetCycleBlocks = 43200; //!< Amount of blocks in a months period of time (using 1 minutes per) = (60*24*30)
        strSporkPubKey = "02efe39918448c329ff03367c8e6ba55d8c868f5edc0fee3c4ea63dd7b92c12ee5";
        strSporkPubKeyOld = "0499A7AF4806FC6DE640D23BC5936C29B77ADF2174B4F45492727F897AE63CF8D27B2F05040606E0D14B547916379FA10716E344E745F880EDC037307186AA25B7";
        strObfuscationPoolDummyAddress = "D87q2gC9j6nNrnzCsg4aY6bHMLsT9nUhEw";
        nStartMasternodePayments = 1403728576; //Wed, 25 Jun 2014 20:36:16 GMT

        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMaxZerocoinPublicSpendsPerTransaction = 637; // Assume about 220 bytes each input
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 1;
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        nZerocoinHeaderVersion = 4; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 999999999; //The required confirmations for a zbtcu to be stakable

        nBudget_Fee_Confirmations = 6; // Number of confirmations for the finalization fee
        nProposalEstablishmentTime = 60 * 60 * 24; // Proposals must be at least a day old to make it into a budget

        //////////qtum
        nMPoSRewardRecipients = 10;
        
        vExcludedAddresses = {"35hK24tcLEWcgNA4JxpvbkNkoAcDGqQPsP",
                              "3Cbq7aT1tY8kMxWLbitaG7yT6bPbKChq64",
                              "1Q7STsw9UzG92BB9qQSs8QNTvni74gVtFG",
                              "1CAdFckxw4o1HbhHD6VZdP68RAURvi8mbC",
                              "1L6oeJ5JVHaSQ4gEQXjvyNGPUdk34magKG",
                              "18PMTSpccrmkgKXPcbqJ3C9A3YTNs6owbm",
                              "18DUJaeuJ8bYjqjrybi48us2dGikSngnrR",
                              "13sAgarPMUPuNkJdRCNr8FiqZrP7RiYwNA",
                              "1H7A7JzChg7KzgQPJ8oHQT1prCSgcBftXa",
                              "1L1xSXttdsBAPVjVfyoyCg3RZbdHinT5G5",
                              "1LAnF8h3qMGx3TSwNUHVneBZUEpwE4gu3D",
                              "1MW8QJahh2YXovXpbhDcXAN8MEEL2AUn4n",
                              "1MAuu297HnLfnCpU3pZJFiLT8HryK3MwLV",
                              "1BKN5obhkdoequshHnn96zZvFi3wCEdfiC",
                              "1Ms2exLWXnXPx3i8za1AVu4mQkqS1fy9W6",
                              "19A8mj9FfW54v1X6wxvHnqS5qVthMT1RsJ",
                              "1E1TB6uxjaEL2N7CJfHkYrA5vwVLa3KfvL",
                              "1WiXrjTFWknSekam88r4Tq1YKNbTZonZo",
                              "1HckjUpRGcrrRAtFaaCAUaGjsPx9oYmLaZ",
                              "18EoFYuD7C37djm6FZi5Csf4AoUtk1yTH3",
                              "19KedreX9aR64fN7tnNzVLVFHQAUL6dLzr",
                              "1LEeT8K9oNdDSfetnyff86qaE5h3ahUEQG",
                              "13dYQ3Vud9jV8e7P7nCqXNvUJ4xkXce1Tt",
                              "1AmajNxtJyU7JjAuyiFFkqDaaxuYqkNSkF",
                              "1GroGcCQfAnL12pLTzGwBDCJ4vbEeypWVd",
                              "1P5ZEDWTKTFGxQjZphgWPQUpe554WKDfHQ",
                              "18vpRm9gLhjhZVdXcEvVgGsop9JWrjDGSG",
                              "1GVzumGXyDyS1dUrQ1TjE82CouGdRWcyz9",
                              "1LQLFiem5HDtjnPL92YeSU3pwvYxtBV8sT",
                              "13eSjPPdKEBSSBktayGBmcYAk4zZ2LZnuT",
                              "bc1q4jchcr7nla277su5lpjzttxp3xg5j8wds5lcwg",
                              "1Fq2zEzWqRPWCwHs1RnvUcx8pGYFcjdLtB",
                              "17xTTMaeW6aGCZMhrCDB8FgfRaeEZbMQyW",
                              "1AotsYu7cMiS6c9KM8dcT6mk1AQS8PmYxV",
                              "121U8Bh6BthG2C8gMhzAwppGdzDjb6nBRX",
                              "1NdR9KZagXjCuQZ2Vhv9zvPenyp3RkYq3C",
                              "12tM672QgT5H49kb6cdzwoHsZ2X3CrfFYy",
                              "12Twhqz9QpHUwtTg9oZdnV2B91DELt1GoW",
                              "1AKbgcGu6dEmPmipzeBuQvfmVknd7gyCnM",
                              "1GMXB5zcf2wooPPS3MyhAeyA1UdK9RcNuF",
                              "bc1qgdjqv0av3q56jvd82tkdjpy7gdp9ut8tlqmgrpmv24sq90ecnvqqjwvw97",
                              "3JZq4atUahhuA9rLhXLMhhTo133J9rF97j",
                              "1Kr6QSydW9bFQG1mXiPNNu6WpJGmUa9i1g",
                              "16FSBGvQfy4K8dYvPPWWpmzgKM6CvrCoVy",
                              "385cR5DM96n1HvBDMzLHPYcw89fZAXULJP",
                              "1N52wHoVR79PMDishab2XmRHsbekCdGquK",
                              "bc1qajh7jfy44sswutqlwj2tz9wcejpxrgflc8penw",
                              "3EKJz9SV3XJmgS3wkmW73tkxhK17j64vct",
                              "3Nxwenay9Z8Lc9JBiywExpnEFiLp6Afp8v",
                              "bc1qx9t2l3pyny2spqpqlye8svce70nppwtaxwdrp4",
                              "12ib7dApVFvg82TXKycWBNpN8kFyiAN1dr",
                              "3Kzh9qAqVWQhEsfQz7zEQL1EuSx5tyNLNS",
                              "1aXzEKiDJKzkPxTZy9zGc3y1nCDwDPub2",
                              "3GQTrVEaRhgGZJzutYhuCsc884WVsd27nq",
                              "3HwHGHcR65mv6zvFvx3RmpBSyT5iUJL85r",
                              "3BMEXqGpG4FxBA1KWhRFufXfSTRgzfDBhJ",
                              "38UmuUqPCrFmQo4khkomQwZ4VbY2nZMJ67",
                              "1MDq7zyLw6oKichbFiDDZ3aaK59byc6CT8",
                              "3DVJfEsDTPkGDvqPCLC41X85L1B1DQWDyh",
                              "34xp4vRoCGJym3xR7yCVPFHoCNxv4Twseo",
                              "1NDyJtNTjmwk5xPNhjgAMu4HDHigtobu1s",
                              "3LCGsSmfr24demGvriN4e3ft8wEcDuHFqh",
                              "336xGpGweq1wtY4kRTuA4w6d7yDkBU9czU",
                              "3EiEN1JJCudBgAwL7c6ajqnPzx9LrK1VT6",
                              "19iqYbeATe4RxghQZJnYVFU4mjUUu76EA6",
                              "1J1F3U7gHrCjsEsRimDJ3oYBiV24wA8FuV",
                              "1AnwDVbwsLBVwRfqN2x9Eo4YEJSPXo2cwG",
                              "14eQD1QQb8QFVG8YFwGz7skyzsvBLWLwJS",
                              "1Kd6zLb9iAjcrgq8HzWnoWNVLYYWjp3swA",
                              "17CzhFvGwH6TT46JtzhnhMpTw4mHYpEGCR",
                              "1PNJfc9MzeBGV1UAjegqhFJb3erejqKHCi",
                              "16iqmhUox1Vh8yjR71SyuYRfP8oRwXspW9",
                              "12fXB6sBJCGbmf7XhRvTXMDCu91329aven",
                              "19Wa69eDP1F6J2Segjsd8co7bVjdLFeL2n",
                              "12gr1kbYQU2M81QBUQ5uAdwA9jKT6oDwx5",
                              "36EHjjQ2sh5KB1pcjXjtBYAaUjfnANavhs",
                              "18EtP9azZCFZgjdNdTSurtNEWF8AEjj8Cs",
                              "3JJdNaku1CUW7Xc4hEytkSs5Y2jmxgmBVX",
                              "17QVYyUMgWzZaTBHgXeA5hM5tnWHXtEmnQ",
                              "169zLUVW2hwqhfaxas9sFaMndRYq5CF6PM",
                              "35vwSVYBZgpxGT3AqjAunHmPHdfvjLWDLY",
                              "3JhjT61XfQZPGAh7Mkj5mrLuv4uG36X3DN",
                              "1Ed6rraaUdKTSGXdkHUw2hVF75469dz77C",
                              "1D32tY21kqGhMnipxP8LLuX4g3o3QbcrK7",
                              "18ZHDyqpcizhEf7GJCgL4XrToXAy38XvUk",
                              "12oMwT4TuDmPe28jJRAw7ox5Pp6t2CSe5H",
                              "14m6BwmNjxrueKwCUA2fE8hkcKcdtdUium",
                              "1A7znRYE24Z6K8MCAKXLmEvuS5ixzvUrjH",
                              "1CXsUumeVbi6SqJ9bWEeC7D8b24PNLxaJv",
                              "1PVgNiKcqXGs7DVxSE2zZE9kKu3GLGL3uv",
                              "1N3dwNTnxDY2mSJTvU3vEGbPGUqPM2W5yx",
                              "18Zcyxqna6h7Z7bRjhKvGpr8HSfieQWXqj",
                              "3Caz8UPikPGR77vJac7ncPGzVbRjRNKMQF",
                              "1DBDgGuZwN2LzeB2ahFRfAZemQpciqSopc",
                              "34DKfu84kAKVKwSsvnqJL1TzspeLTRSoZo",
                              "37vPwvYPFk12KNTtDNZkKMv7qxx2adHHPA",
                              "3NDuGtLTjBEF1yj5KpsqT7C6EQhhNM4E6n",
                              "1NQXknw7x3ubk3SuXZJQXZgGH3GUS53Xb8",
                              "3KTQYXvjteNoMECi62JYuqXobYQpcHjoVs",
                              "1BKapnuL2HJ7VH839Kcq8nJYC18ya8QHVg",
                              "19Z3UUqMMCwEifofoAc7epqNeP2t556Fte",
                              "3GKzCD4cQFqstTPseYp1j3CVsh8KNfHtjm",
                              "1JAkRY3Tbegff9ZLUNgPSNYgJv2fUNbRMK",
                              "35pgGeez3ou6ofrpjt8T7bvC9t6RrUK4p6",
                              "1NsCi66rAsp49pnxMQxfKT7XHpAK7q9mZ3",
                              "3K7Hv5DgZ6QK8peZbh2T9LU24495J8LUht",
                              "16SgQsyge2ov79B4Ncp6DQfKNSxwE2hzsi",
                              "396x7FJ3J9K1M6HaKeiWNyEf84aKFBSbxL",
                              "3BTTDAn8HrmS2Lx48EoJy6v35B4jvAUW8p",
                              "1MYkFz8bXgnfWpdE7cHFwFqaULV4o8xMJQ",
                              "3NrWT4hyAzcqBxZrF6TSDWbYpBktovBWoV",
                              "3HosKm4oiSWdnHC2By4MfYzWzccRHqRYgn",
                              "bc1quq29mutxkgxmjfdr7ayj3zd9ad0ld5mrhh89l2",
                              "1NYAd6fA2dc5xowuweFUSDRqRTEzDwk28",
                              "19TNyoWBB2VzXb14YoDFfsoDhu9D74j5RW",
                              "1FeexV6bAHb8ybZjqQMjJrcCrHGW9sb6uF",
                              "1HTzd3sKVmrTNZ6QGisPPA1MvBkoyPiZWJ",
                              "12cgpFdJViXbwHbhrA3TuW1EGnL25Zqc3P",
                              "17A16QmavnUfCW11DAApiJxp7ARnxN5pGX",
                              "17ac9tXHxu1nxdLgLu9WYk7vR8ggFN5GkH",
                              "37XuVSEpWW4trkfmvWzegTHQt7BdktSKUs",
                              "16SLJL6RCqHjySsKEdaEnNRWTrMhv6S8Z5",
                              "1KFHE7w8BhaENAswwryaoccDb6qcT6DbYY",
                              "1LU4sSfLAukDcbJim9dvNKELHxk4YMSnW1",
                              "1M3Nn3Znq2MVqjDaoR1GucVYmXvXLgYPz4",
                              "1Gpn6EG28vX2fyoKSQANCoUMc3ZGMtbWF9",
                              "1aXp4NfaT9vb8yuvu78raWfLN52PBe1xQ",
                              "3QkHGNXtWX9CHkRz9QnwYnC3eeCv6qc6ji",
                              "1AgyKRw9DAf4BjSBQBAMn8UFJYnKxkMypK",
                              "1PhtuY4vR4vrCqDSSiXXEwFmjsvKbW2in9",
                              "3BMEXxSMT2b2kvsnC4Q35d2kKJZ4u9bSLh",
                              "15Z5YJaaNSxeynvr6uW6jQZLwq3n1Hu6RX",
                              "1A1QkRJfBKZkf8NhYPH9TtcCi99bp2TVQ6"
        };

        rechargedAddress = "1P8pv9WdDiy9LSyU9Z2DzuEEz74LwRxsqs";
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x45;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xba;
        vAlertPubKey = ParseHex("0424f2affc27906832348adacf23f475a2a758c7d9ed4510d209a05f921653aaa1a6c3bb0683d91964cab3c12d868813e2f5835091f53a3c0a06e1afa057b9530a");
        nDefaultPort = 13666;
        nEnforceBlockUpgradeMajority = 4320; // 75%
        nRejectBlockOutdatedMajority = 5472; // 95%
        nToCheckBlockUpgradeMajority = 5760; // 4 days
        nMinerThreads = 0;
        nLastPOWBlock = 0;
        nBtcuBadBlockTime = 1489001494; // Skip nBit validation of Block 259201 per PR #915
        nBtcuBadBlocknBits = 0x1e0a20bd; // Skip nBit validation of Block 201 per PR #915
        nMaturity = 5; // 3 validators in testnet, for  activatin pos must be one more
        nStakeMinDepth = 20;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nZerocoinStartHeight = 1;
        nZerocoinStartTime = 1501776000;
        nBlockEnforceSerialRange = 1; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 1; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 9891737; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 9891730; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 9902850; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = 444020; //!> The block that zerocoin v2 becomes active
        nEnforceNewSporkKey = 1566860400; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
        nRejectOldSporkKey = 1569538800; //!> Reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
        nBlockStakeModifierlV2 = nLastPOWBlock + 1;
        nBIP65ActivationHeight = 851019;
        // Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
        nBlockTimeProtocolV2 = 1347000;

        // Public coin spend enforcement
        nPublicZCSpends = 0;

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

        // Blocks v7
        nBlockLastAccumulatorCheckpoint = nPublicZCSpends - 10;
        nBlockV7StartHeight = nBlockTimeProtocolV2;

        // Leasing
        nLeasingRewardMaturity = 3; // 3 blocks
        nLeasingRewardPeriod = 10; // 10 minutes
        nMaxLeasingRewards = 10;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = -1;
        nSupplyBeforeFakeSerial = 0;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1583491266;
        genesis.nNonce = 8424874;

        //! Modify genesis testnet validators pubkeys
        CMutableTransaction txNew = genesis.vtx[0];
        txNew.validatorRegister.clear();

        std::vector<std::string> validatorsPubkeys = {
        "029b5a66fdd5f0e37c42e0fd5fba64b5688635724c4d6ec8f51ca8875491bbdbe3",
        "03d176c15d7e634eef71091c7c552fbed35a990bd524160837b6802b549033ce7c",
        "02796b6e1dee823a1ec48a05eaad873eabd11deda696bc0350b5d23f8d0ca4b8f3"
        };

        for(int i = 0; i < validatorsPubkeys.size(); i++)
        {
           CTxIn validatorVin(uint256(0), i);
           CPubKey validatorPubKey(ParseHex(validatorsPubkeys[i].data()));

           CValidatorRegister validatorReg(validatorVin, validatorPubKey);
           validatorReg.nTime = 0;
           txNew.validatorRegister.push_back(validatorReg);
        }
        genesis.vtx[0] = txNew;
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x00000c6a7006cd1a4c53ada0d162cd1b946226742e4ee6815b92c19eeda12f23"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111); // (Bitcoin defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196); // (Bitcoin defaults)
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1,53);  // starting with 'N'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tb";

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        nBudgetCycleBlocks = 144; //!< Ten cycles per day on testnet
        strSporkPubKey = "026fe3671df2061f611fe7c3048bb33a1ade7a06f56f409a957605d115209a0f25";
        strSporkPubKeyOld = "04A8B319388C0F8588D238B9941DC26B26D3F9465266B368A051C5C100F79306A557780101FE2192FE170D7E6DEFDCBEE4C8D533396389C0DAFFDBC842B002243C";
        strObfuscationPoolDummyAddress = "y57cqfGRkekRyDRNeJiLtYVEbvhXrNbmox";
        nStartMasternodePayments = 1420837558;
        nBudget_Fee_Confirmations = 3; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet

        nProposalEstablishmentTime = 60 * 5; // Proposals must be at least 5 mns old to make it into a test budget
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 23666;
        nSubsidyHalvingInterval = 150;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        nLastPOWBlock = 250;
        nMaturity = 100;
        nStakeMinAge = 0;
        nStakeMinDepth = 0;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 0;
        nZerocoinStartHeight = 300;
        nBlockZerocoinV2 = 300;
        nZerocoinStartTime = 1501776000;
        nBlockEnforceSerialRange = 1;               // Enforce serial range starting this block
        nBlockRecalculateAccumulators = 999999999;  // Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 999999999;          // First block that bad serials emerged
        nBlockLastGoodCheckpoint = 999999999;       // Last valid accumulator checkpoint
        nBlockStakeModifierlV2 = nLastPOWBlock + 1; // start with modifier V2 on regtest
        nBlockTimeProtocolV2 = 999999999;

        nMintRequiredConfirmations = 10;
        nZerocoinRequiredStakeDepth = nMintRequiredConfirmations;

        // Public coin spend enforcement
        nPublicZCSpends = 400;

        // Blocks v7
        nBlockV7StartHeight = nBlockZerocoinV2;
        nBlockLastAccumulatorCheckpoint = nBlockZerocoinV2+1; // no accumul. checkpoints check on regtest

        // New P2P messages signatures
        nBlockEnforceNewMessageSignatures = 1;

        // Fake Serial Attack
        nFakeSerialBlockheightEnd = -1;
        
        //! Modify the regtest genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1583491266;
        genesis.nNonce = 5273717;

        //! Modify genesis regtest validators pubkeys
        CMutableTransaction txNew = genesis.vtx[0];
        txNew.validatorRegister.clear();

        std::vector<std::string> validatorsPubkeys = {
        "0268cd814b0b32555741fdb586110e48f9ce100e5805296907d89a179358be6d47"
        };

        for(int i = 0; i < validatorsPubkeys.size(); i++)
        {
           CTxIn validatorVin(uint256(0), i);
           CPubKey validatorPubKey(ParseHex(validatorsPubkeys[i].data()));

           CValidatorRegister validatorReg(validatorVin, validatorPubKey);
           validatorReg.nTime = 0;
           txNew.validatorRegister.push_back(validatorReg);
        }
        genesis.vtx[0] = txNew;
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x00000e122e0b667d0f81ce3aa511f272fd1cf3a8692552c9afa24e48ff189120"));
        
        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";

        bech32_hrp = "bcrt";
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

static CChainParams* pCurrentParams = 0;

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

std::string CChainParams::EVMGenesisInfo(dev::eth::Network network) const
{
    // replace_constants
    std::string genesisInfo = dev::eth::genesisInfo(network);
    ReplaceInt(0, "QIP7_STARTING_BLOCK", genesisInfo);
    ReplaceInt(0, "QIP6_STARTING_BLOCK", genesisInfo);
    return genesisInfo;
}

std::string CChainParams::EVMGenesisInfo() const
{
   dev::eth::EVMConsensus evmConsensus;
   evmConsensus.QIP6Height = 0;
   evmConsensus.QIP7Height = 0;
   evmConsensus.nMuirGlacierHeight = 0;
   evmConsensus.nLondonHeight = 0;
   return dev::eth::genesisInfoQtum(dev::eth::Network::qtumNetwork, evmConsensus);
}

std::string toHexString(int64_t intValue) {
    //Store big endian representation in a vector
    uint64_t num = (uint64_t)intValue;
    std::vector<unsigned char> bigEndian;
    for(int i=sizeof(num) -1; i>=0; i--){
        bigEndian.push_back( (num>>(8*i)) & 0xff );
    }

    //Convert the vector into hex string
    return "0x" + HexStr(bigEndian.begin(), bigEndian.end());
}

void ReplaceInt(const int64_t& number, const std::string& key, std::string& str)
{
    // Convert the number into hex string
    std::string num_hex = toHexString(number);

    // Search for key in str and replace it with the hex string
    std::string str_replaced = std::regex_replace(str, std::regex(key), num_hex);
    str = str_replaced;
}