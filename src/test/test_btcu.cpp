// Copyright (c) 2011-2013 The Bitcoin Core developers
// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Btcu Test Suite

#include "test_btcu.h"

#include "main.h"
#include "random.h"
#include "txdb.h"
#include "guiinterface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet/db.h"
#include "wallet/wallet.h"
#endif

#include <boost/test/included/unit_test.hpp>

extern CWallet* pwalletMain;

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();

BasicTestingSetup::BasicTestingSetup()
{
        RandomInit();
        ECC_Start();
        SetupEnvironment();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(CBaseChainParams::MAIN);
}
BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup() : BasicTestingSetup()
{
#ifdef ENABLE_WALLET
        bitdb.MakeMock();
#endif
        ClearDatadirCache();
        pathTemp = GetTempPath() / strprintf("test_btcu_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        boost::filesystem::create_directories(pathTemp);
        mapArgs["-datadir"] = pathTemp.string();
        pblocktree = new CBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CCoinsViewDB(1 << 23, true);
        pcoinsTip = new CCoinsViewCache(pcoinsdbview);
        InitBlockIndex();

        namespace fs = boost::filesystem;
        const CChainParams& chainparams = Params();
        dev::eth::NoProof::init();
        fs::path qtumStateDir = GetDataDir() / "stateQtum";
        bool fStatus = fs::exists(qtumStateDir);
        const std::string dirQtum(qtumStateDir.string());
        const dev::h256 hashDB(dev::sha3(dev::rlp("")));
        dev::eth::BaseState existsQtumstate = fStatus ? dev::eth::BaseState::PreExisting : dev::eth::BaseState::Empty;
        globalState = std::unique_ptr<QtumState>(new QtumState(dev::u256(0), QtumState::openDB(dirQtum, hashDB, dev::WithExisting::Trust), dirQtum, existsQtumstate));
        auto geni = chainparams.EVMGenesisInfo(dev::eth::Network::qtumNetwork);
        dev::eth::ChainParams cp((geni));
        globalSealEngine = std::unique_ptr<dev::eth::SealEngineFace>(cp.createSealEngine());
        globalState->setRoot(dev::sha3(dev::rlp("")));
        globalState->setRootUTXO(uintToh256(chainparams.GenesisBlock().hashUTXORoot));
        globalState->populateFrom(cp.genesisState);

        globalState->db().commit();
        globalState->dbUtxo().commit();

   {
            CValidationState state;
            bool ok = ActivateBestChain(state);
            BOOST_CHECK(ok);
        }


#ifdef ENABLE_WALLET
        bool fFirstRun;
        pwalletMain = new CWallet("wallet.dat");
        pwalletMain->LoadWallet(fFirstRun);
        auto zwalletMain = new CzBTCUWallet(pwalletMain->strWalletFile);
        pwalletMain->setZWallet(zwalletMain);
        RegisterValidationInterface(pwalletMain);
#endif
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        RegisterNodeSignals(GetNodeSignals());

}

TestingSetup::~TestingSetup()
{
        UnregisterNodeSignals(GetNodeSignals());
        threadGroup.interrupt_all();
        threadGroup.join_all();
#ifdef ENABLE_WALLET
        UnregisterValidationInterface(pwalletMain);
        delete pwalletMain;
        pwalletMain = NULL;
#endif
        UnloadBlockIndex();
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
#ifdef ENABLE_WALLET
        bitdb.Flush(true);
        bitdb.Reset();
#endif
        boost::filesystem::remove_all(pathTemp);
}
