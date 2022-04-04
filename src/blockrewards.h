//
// Copyright (c) 2021 The BTCU developers
//

#ifndef BITCOIN_ULTIMATUM_BLOCKREWARDS_H
#define BITCOIN_ULTIMATUM_BLOCKREWARDS_H

#include "main.h"

int64_t GetBlockValue(int nHeight)
{
    if (Params().NetworkID() == CBaseChainParams::TESTNET) {
        if (nHeight < 200 && nHeight > 0)
            return 250000 * COIN;
    }

    if (Params().IsRegTestNet()) {
        if (nHeight == 0)
            return 250 * COIN;

    }

    int64_t nSubsidy = 0;
    if (nHeight >= 0 && nHeight <= 30000)              { nSubsidy = 0.002  * COIN;  }
    else if (nHeight >= 30001   && nHeight <= 90000)   { nSubsidy = 0.0018 * COIN; }
    else if (nHeight >= 90001   && nHeight <= 180000)  { nSubsidy = 0.0016 * COIN; }
    else if (nHeight >= 180001  && nHeight <= 270000)  { nSubsidy = 0.0014 * COIN; }
    else if (nHeight >= 270001  && nHeight <= 360000)  { nSubsidy = 0.0012 * COIN; }
    else if (nHeight >= 360001  && nHeight <= 520000)  { nSubsidy = 0.001 * COIN; }
    else if (nHeight >= 520001  && nHeight <= 720000)  { nSubsidy = 0.0008 * COIN; }
    else if (nHeight >= 720001  && nHeight <= 1080000) { nSubsidy = 0.0006 * COIN; }
    else if (nHeight >= 1080001 && nHeight <= 1440000) { nSubsidy = 0.0004 * COIN; }
    else if (nHeight >= 1440001 && nHeight <= 1800000) { nSubsidy = 0.0002 * COIN; }
    else if (nHeight >= 1800001)                       { nSubsidy = 0;}
    return nSubsidy;
}

int64_t GetMasternodePayment(int nHeight, int64_t blockValue, int nMasternodeCount, bool isZBTCUStake)
{
   int64_t ret = 0;
   if (nHeight >= 0 && nHeight <= 30000)              { ret = 0.001  * COIN;  }
   else if (nHeight >= 30001   && nHeight <= 90000)   { ret = 0.0009 * COIN; }
   else if (nHeight >= 90001   && nHeight <= 180000)  { ret = 0.0008 * COIN; }
   else if (nHeight >= 180001  && nHeight <= 270000)  { ret = 0.0007 * COIN; }
   else if (nHeight >= 270001  && nHeight <= 360000)  { ret = 0.0006 * COIN; }
   else if (nHeight >= 360001  && nHeight <= 520000)  { ret = 0.0005 * COIN; }
   else if (nHeight >= 520001  && nHeight <= 720000)  { ret = 0.0004 * COIN; }
   else if (nHeight >= 720001  && nHeight <= 1080000) { ret = 0.0003 * COIN; }
   else if (nHeight >= 1080001 && nHeight <= 1440000) { ret = 0.0002 * COIN; }
   else if (nHeight >= 1440001 && nHeight <= 1800000) { ret = 0.0001 * COIN; }
   else if (nHeight >= 1800001)                       { ret = 0;}
   return ret;
}
#endif //BITCOIN_ULTIMATUM_BLOCKREWARDS_H
