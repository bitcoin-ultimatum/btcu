// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2018 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/validatorwidget.h"
#include "qt/btcu/forms/ui_validatorwidget.h"
#include "qt/btcu/qtutils.h"
#include "qt/btcu/mnrow.h"

#include "clientmodel.h"
#include "guiutil.h"
#include "init.h"
#include "wallet/wallet.h"
#include "walletmodel.h"
#include "util.h"
#include "qt/btcu/optionbutton.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "qt/btcu/createvalidatorwidget.h"

#include "qt/btcu/defaultdialog.h"
#include "qt/btcu/validatorrow.h"
#include "main.h"
#include "registervalidator.h"
#include <QGraphicsDropShadowEffect>
#include "../rpc/validators.cpp"

#define DECORATION_SIZE 65
#define NUM_ITEMS 3

ValidatorWidget::ValidatorWidget(BTCUGUI *parent) :
    PWidget(parent),
    ui(new Ui::ValidatorWidget),
    isLoading(false)
{
    ui->setupUi(this);

   /* delegate = new FurAbstractListItemDelegate(
            DECORATION_SIZE,
            new MNHolder(isLightTheme()),
            this
    );
    mnModel = new MNModel(this);*/

    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    //setCssProperty(ui->left, "container");
    //ui->left->setContentsMargins(0,20,0,20);
    //setCssProperty(ui->right, "container-right");
    //ui->right->setContentsMargins(20,20,20,20);
   //ui->right->setVisible(false);
   setCssProperty(ui->scrollAreaMy, "container");
   this->setGraphicsEffect(0);
   //ui->scrollAreaMy->setGraphicsEffect(0);
    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText(tr("Validators"));
    setCssTitleScreen(ui->labelTitle);
    ui->labelTitle->setFont(fontLight);

   setCssProperty(ui->labelName, "text-body2-grey");
   setCssProperty(ui->labelPrivateKey, "text-body2-grey");
   setCssProperty(ui->labelVIN, "text-body2-grey");

   setCssProperty(ui->pushImgEmpty, "img-empty-master");
   ui->labelEmpty->setText(tr("No active Masternode yet"));
   setCssProperty(ui->labelEmpty, "text-empty");

    //ui->labelSubtitle1->setText(tr("Full nodes that incentivize node operators to perform the core consensus functions \n and vote on trasury system receiving a periodic reward."));
    //setCssSubtitleScreen(ui->labelSubtitle1);

    /* Buttons */
   ui->pbnRegistered->setProperty("cssClass","btn-check-left");
   ui->pbnRegistered->setChecked(true);
   ui->pbnVote->setProperty("cssClass","btn-check-left");
   ui->pbnVote->setChecked(false);
   ui->pbnActive->setProperty("cssClass","btn-check-left");
   ui->pbnActive->setChecked(false);
   ui->pbnValidator->setProperty("cssClass","btn-secundary-small");
   ui->pushButtonVote->setProperty("cssClass","btn-secundary-small");

   connect(ui->pbnValidator, SIGNAL(clicked()), this, SLOT(onpbnValidatorClicked()));
   connect(ui->pbnVote, SIGNAL(clicked()), this, SLOT(onpbnVoteClicked()));
   connect(ui->pbnRegistered, SIGNAL(clicked()), this, SLOT(onpbnRegisteredClicked()));
   connect(ui->pbnActive, SIGNAL(clicked()), this, SLOT(onpbnActiveClicked()));
   connect(ui->pushButtonVote, SIGNAL(clicked()), this, SLOT(onPushButtonVote()));
   onpbnRegisteredClicked();
}

void ValidatorWidget::loadWalletModel(){
    /* if(walletModel) {
         ui->listMn->setModel(mnModel);
         ui->listMn->setModelColumn(AddressTableModel::Label);
         updateListState();
     }*/
}

void ValidatorWidget::showHistory()
{
/*
   ui->verticalSpacer_2->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Fixed);
   ui->verticalSpacer_3->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Fixed);
   if(ui->pbnRegistered->isChecked())
   {
      ui->scrollArea->setVisible(false);
      ui->scrollAreaMy->setVisible(true);
      if(bShowHistoryMy)
      {
         ui->NodesContainer->setVisible(true);
         ui->emptyContainer->setVisible(false);
      }
      else
      {
         ui->verticalSpacer_2->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding);
         ui->verticalSpacer_3->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding);
         ui->NodesContainer->setVisible(false);
         ui->emptyContainer->setVisible(true);
      }
   }
   else{
      ui->scrollArea->setVisible(true);
      ui->scrollAreaMy->setVisible(false);
      if(bShowHistory)
      {
         ui->NodesContainer->setVisible(true);
         ui->emptyContainer->setVisible(false);
      }
      else
      {
         ui->verticalSpacer_2->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding);
         ui->verticalSpacer_3->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding);
         ui->NodesContainer->setVisible(false);
         ui->emptyContainer->setVisible(true);
      }
   }
*/
}

void ValidatorWidget::onpbnValidatorClicked()
{
    if(!walletModel)
    {
        informError(tr("Wallet model does not load."));
        return;
    }
   RegisterValidator* newValidator = new RegisterValidator(walletModel,this);
   //newValidator->setTableModel(walletModel);
   connect(newValidator, SIGNAL(registered(std::string, std::string, std::string)), this, SLOT(onRegisterValidator(std::string, std::string, std::string)));
   newValidator->setWalletModel(walletModel);
   showHideOp(true);
   newValidator->show();
   //openDialogWithOpaqueBackground(newValidator, window);
   //window->goToCreateValidator();

   //open widget with opaque background
    newValidator->setWindowFlags(Qt::CustomizeWindowHint);
    newValidator->setAttribute(Qt::WA_TranslucentBackground, true);

    newValidator->activateWindow();
    newValidator->resize(window->width(),window->height());

    QPropertyAnimation* animation = new QPropertyAnimation(newValidator, "pos");
    animation->setDuration(300);
    int xPos = 0;
    animation->setStartValue(QPoint(xPos, window->height()));
    animation->setEndValue(QPoint(xPos, 0));
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    newValidator->activateWindow();
    newValidator->show();
    window->showHide(false);
}

void ValidatorWidget::onpbnRegisteredClicked()
{
    showRowsOfValidators(false);

    ui->pbnRegistered->setChecked(true);
    ui->pbnVote->setChecked(false);
    ui->pbnActive->setChecked(false);
    ui->pushButtonVote->hide();
    showHistory();
}

void ValidatorWidget::onpbnVoteClicked()
{
    showRowsOfValidators(true);

    ui->pbnRegistered->setChecked(false);
    ui->pbnVote->setChecked(true);
    ui->pbnActive->setChecked(false);
    ui->pushButtonVote->show();
    showHistory();

}

void ValidatorWidget::showRowsOfValidators(bool isVote)
{
    if(ui->pbnActive->isChecked())
        for (auto iter : activeValidatorsRowVector)
            iter->setVisible(false);
    for (auto iter : registeredValidatorsRowVector)
    {
        iter->setVisible(true);
        iter->setVoteVisible(isVote);
    }

    auto validatorsRegistrationList = g_ValidatorsState.get_registrations();
    std::string pubKey;
    std::string vin;
    std::string name;

    for (auto &valReg : validatorsRegistrationList)
    {
        if(registeredValidators.size() == 0)
        {
            savingNewValidator(HexStr(valReg.pubKey), valReg.vin, isVote);
        }
        for (int i = 0; i < registeredValidators.size(); ++i) {
            if (registeredValidators[i].pubKey != HexStr(valReg.pubKey))
            {
                if (SpacerNodeMy) {
                    ui->scrollAreaWidgetContentsMy->layout()->removeItem(SpacerNodeMy);
                    delete SpacerNodeMy;
                }

                savingNewValidator(HexStr(valReg.pubKey), valReg.vin, isVote);
            }
        }
    }
//only for test (if registeredValidatorsRowVector is empty) delete later
    if(registeredValidatorsRowVector.size() == 0)
    {
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
        shadowEffect->setColor(QColor(0, 0, 0, 22));
        shadowEffect->setXOffset(0);
        shadowEffect->setYOffset(2);
        shadowEffect->setBlurRadius(6);

        SpacerNodeMy = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        std::shared_ptr<ValidatorRow> validatorRow = std::make_shared<ValidatorRow>(ui->scrollAreaMy);
        validatorRow->setGraphicsEffect(shadowEffect);
        //validatorRow->updateView(QString::fromStdString(name), QString::fromStdString(pubKey), QString::fromStdString(vin));
        connect(validatorRow.get(), SIGNAL(voted(QString, bool)), this, SLOT(onVoted(QString, bool)));
        validatorRow->setVoteVisible(isVote);
        registeredValidatorsRowVector.push_back(validatorRow);
        ui->scrollAreaWidgetContentsMy->layout()->addWidget(validatorRow.get());
        ui->scrollAreaWidgetContentsMy->layout()->addItem(SpacerNodeMy);
    }
}

void ValidatorWidget::savingNewValidator(std::string pubKey, CTxIn vin, bool isVote) {
    std::cout << pubKey << " " << vin.ToString() << std::endl << std::endl;

    registeredValidators.push_back({"", pubKey, vin, false});

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setColor(QColor(0, 0, 0, 22));
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(2);
    shadowEffect->setBlurRadius(6);

    SpacerNodeMy = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    std::shared_ptr<ValidatorRow> validatorRow = std::make_shared<ValidatorRow>(ui->scrollAreaMy);
    validatorRow->setGraphicsEffect(shadowEffect);
    //validatorRow->updateView(QString::fromStdString(name), QString::fromStdString(pubKey), QString::fromStdString(vin));
    connect(validatorRow.get(), SIGNAL(voted(QString, bool)), this, SLOT(onVoted(QString, bool)));
    validatorRow->setVoteVisible(isVote);
    registeredValidatorsRowVector.push_back(validatorRow);
    ui->scrollAreaWidgetContentsMy->layout()->addWidget(validatorRow.get());
    ui->scrollAreaWidgetContentsMy->layout()->addItem(SpacerNodeMy);
}

void ValidatorWidget::onpbnActiveClicked()
{
    ui->pbnRegistered->setChecked(false);
    ui->pbnVote->setChecked(false);
    ui->pbnActive->setChecked(true);
    ui->pushButtonVote->hide();
    showHistory();

    for (auto iter : registeredValidatorsRowVector)
        iter->setVisible(false);

    for (auto iter : activeValidatorsRowVector)
        iter->setVisible(true);

    auto validatorsList = g_ValidatorsState.get_validators();

    for(auto &val : validatorsList)
    {
        if(activeValidators.size() == 0)
        {
            savingActiveValidator(HexStr(val.pubKey), val.vin);
        }
        for (int i = 0; i < activeValidators.size(); ++i) {
            if (activeValidators[i].pubKey != HexStr(val.pubKey))
            {
                savingActiveValidator(HexStr(val.pubKey), val.vin);
            }
        }
    }
//only for test (if activeValidatorsRowVector is empty) delete later
    if(activeValidatorsRowVector.size() == 0)
    {
        if (SpacerNodeMy) {
            ui->scrollAreaWidgetContentsMy->layout()->removeItem(SpacerNodeMy);
            delete SpacerNodeMy;
        }

        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
        shadowEffect->setColor(QColor(0, 0, 0, 22));
        shadowEffect->setXOffset(0);
        shadowEffect->setYOffset(2);
        shadowEffect->setBlurRadius(6);

        SpacerNodeMy = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        std::shared_ptr<ValidatorRow> validatorRow = std::make_shared<ValidatorRow>(ui->scrollAreaMy);
        validatorRow->setGraphicsEffect(shadowEffect);
        //validatorRow->updateView(QString::fromStdString(name), QString::fromStdString(pubKey), QString::fromStdString(vin));
        validatorRow->setVoteVisible(false);
        activeValidatorsRowVector.push_back(validatorRow);
        ui->scrollAreaWidgetContentsMy->layout()->addWidget(validatorRow.get());
        ui->scrollAreaWidgetContentsMy->layout()->addItem(SpacerNodeMy);
    }
}

void ValidatorWidget::savingActiveValidator(std::string pubKey, CTxIn vin)
{
    if (SpacerNodeMy) {
        ui->scrollAreaWidgetContentsMy->layout()->removeItem(SpacerNodeMy);
        delete SpacerNodeMy;
    }

    activeValidators.push_back({"", pubKey, vin, false});

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setColor(QColor(0, 0, 0, 22));
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(2);
    shadowEffect->setBlurRadius(6);

    SpacerNodeMy = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    std::shared_ptr<ValidatorRow> validatorRow = std::make_shared<ValidatorRow>(ui->scrollAreaMy);
    validatorRow->setGraphicsEffect(shadowEffect);
    //validatorRow->updateView(QString::fromStdString(name), QString::fromStdString(pubKey), QString::fromStdString(vin));
    validatorRow->setVoteVisible(false);
    activeValidatorsRowVector.push_back(validatorRow);
    ui->scrollAreaWidgetContentsMy->layout()->addWidget(validatorRow.get());
    ui->scrollAreaWidgetContentsMy->layout()->addItem(SpacerNodeMy);
}

void ValidatorWidget::onVoted(QString pubKey, bool vote)
{
    std::cout << pubKey.toStdString() << " " << vote << std::endl;

    for(int i = 0; i < registeredValidators.size(); ++i)
        if(registeredValidators[i].pubKey == pubKey.toStdString()) {
            registeredValidators[i].vote = vote;
            break;
        }
}

void ValidatorWidget::onPushButtonVote()
{
    DefaultDialog *Defdialog = new DefaultDialog(window);
    Defdialog->setText(tr("Are you sure you want to vote?"), tr(""), tr("yes"),tr("no"));
    Defdialog->setType();
    Defdialog->adjustSize();
    showHideOp(true);
    openDialogWithOpaqueBackground(Defdialog, window );
    if(!Defdialog->result())
    {
        informError(tr("Not confirm voting!"));
        return;
    }

    UniValue ret(UniValue::VOBJ);
    int currentPositionInVotingPeriod = (chainActive.Height() + 1) % VALIDATORS_VOTING_PERIOD_LENGTH; // +1 due to current transaction should be included at least into the next block
    // Checking that it is corresponding phase in the current voting period
    if((currentPositionInVotingPeriod >= VALIDATORS_VOTING_START) &&
       (currentPositionInVotingPeriod <= VALIDATORS_VOTING_END))
    {
        std::vector<MNVote> votes;
        votes.reserve(registeredValidators.size());

        for(int i = 0; i < registeredValidators.size(); ++i)
        {
            votes.emplace_back(registeredValidators[i].vin, registeredValidators[i].vote ? 1 : -1);
        }

        boost::optional<CValidatorVote> valVoteOpt = createValidatorVote(votes);
        if (valVoteOpt.is_initialized()){
            ret = createAndSendTransaction(boost::optional<CValidatorRegister>(), valVoteOpt, "");
        }
        {
            informError(tr("Failed to get validator masternode key: Validator vote creation failed. Check your masternode status."));
            return;
        }
    }
    else
    {
        QString error = QString::number(VALIDATORS_VOTING_PERIOD_LENGTH - currentPositionInVotingPeriod) + " blocks until start of the registration phase";
        informError(tr(error.toStdString().c_str()));
        return;
    }

}

boost::optional<CValidatorVote> ValidatorWidget::createValidatorVote(const std::vector<MNVote> &votes)
{
    boost::optional<CValidatorVote> valVoteOpt;

    auto keyOpt = getRegisteringValidatorVinKey();
    if(keyOpt.is_initialized())
    {
        auto vinKey = keyOpt.value();

        CValidatorVote valVote(vinKey.first, vinKey.second.GetPubKey(), votes);
        if(valVote.Sign(vinKey.second))
        {
            valVoteOpt.emplace(valVote);
        }
    }
    return valVoteOpt;
}

boost::optional<std::pair<CTxIn, CKey>> ValidatorWidget::getRegisteringValidatorVinKey()
{
    boost::optional<std::pair<CTxIn, CKey>> vinKeyOpt;

    for (CMasternodeConfig::CMasternodeEntry &mne : masternodeConfig.getEntries())
    {
        CKey keyMasternode;
        CPubKey pubKeyMasternode;
        if(CMessageSigner::GetKeysFromSecret(mne.getPrivKey(), keyMasternode, pubKeyMasternode))
        {
            CMasternode *pmn = mnodeman.Find(pubKeyMasternode);
            if(pmn != nullptr)
            {
                auto keyOpt = getCollateralKey(pmn);
                if(keyOpt.is_initialized())
                {
                    auto validatorsRegistrationList = g_ValidatorsState.get_registrations();
                    for(auto &validator: validatorsRegistrationList)
                    {
                        if(validator.pubKey == keyOpt.value().GetPubKey())
                        {
                            vinKeyOpt.emplace(std::pair<CTxIn, CKey>(pmn->vin, keyOpt.value()));
                            return vinKeyOpt;
                        }
                    }
                }
            }
        }
    }
    return vinKeyOpt;
}

boost::optional<CKey> ValidatorWidget::getCollateralKey(CMasternode *pmn)
{
    boost::optional<CKey> keyOpt;
    CKey key;

    auto addr = pmn->pubKeyCollateralAddress.GetID(); // public key ID for MN's collateral address
    if (pwalletMain->GetKey(addr, key)) // get key (private and public parts) from wallet
    {
        keyOpt.emplace(key);
    }
    return keyOpt;
}

void ValidatorWidget::onRegisterValidator(std::string MNName, std::string address, std::string hash)
{
    std::cout << MNName << " " << address << std::endl << std::endl;

    DefaultDialog *Defdialog = new DefaultDialog(window);
    std::string name = MNName + " validator?";
    Defdialog->setText(tr("Are you sure you want to register"), tr(name.c_str()), tr("yes"),tr("no"));
    Defdialog->setType();
    Defdialog->adjustSize();
    showHideOp(true);
    openDialogWithOpaqueBackground(Defdialog, window );
    if(!Defdialog->result())
    {
        informError(tr("Not confirm validator registration!"));
        return;
    }

    //UniValue ret(UniValue::VOBJ);
    std::string ret;
    int currentPositionInVotingPeriod = (chainActive.Height() + 1) % VALIDATORS_VOTING_PERIOD_LENGTH;  // +1 due to current transaction should be included at least into the next block
    //Checking that it is corresponding phase in the current voting period

    boost::optional<CValidatorRegister> valRegOpt = createValidatorReg(MNName, hash, address);
    ret = createAndSendTransaction(valRegOpt, boost::optional<CValidatorVote>(), hash);

    /*if((currentPositionInVotingPeriod >= VALIDATORS_REGISTER_START) &&
       (currentPositionInVotingPeriod <= VALIDATORS_REGISTER_END)) {
        boost::optional<CValidatorRegister> valRegOpt = CreateValidatorReg(MNName);
        if (valRegOpt.is_initialized()) {
            ret = craeteAndSendTransaction(valRegOpt, boost::optional<CValidatorVote>());
        }
        else
        {
            informError(tr("Validator creation failed"));
            return;
        }
    }
    else
    {
        QString error = QString::number(VALIDATORS_VOTING_PERIOD_LENGTH - currentPositionInVotingPeriod) + " blocks until start of the registration phase";
        informError(tr(error.toStdString().c_str()));
        return;
    }*/
    std::cout << ret << std::endl;
}
boost::optional<CValidatorRegister> ValidatorWidget::createValidatorReg(const std::string &mnName, const std::string &mnHash, const std::string &mnAddress)
{
    boost::optional<CValidatorRegister> valRegOpt;

    auto keyOpt = getVinKey(mnName, mnHash, mnAddress);
    if(keyOpt.is_initialized())
    {
        auto vinKey = keyOpt.value();

        CValidatorRegister valReg(vinKey.first, vinKey.second.GetPubKey());
        if(valReg.Sign(vinKey.second))
        {
            valRegOpt.emplace(valReg);
        }
    }
    return valRegOpt;
}

boost::optional<std::pair<CTxIn, CKey>> ValidatorWidget::getVinKey(const std::string &mnName, const std::string &mnHash, const std::string &mnAddress)
{
    boost::optional<std::pair<CTxIn, CKey>> vinKeyOpt;
    std::cout << "hash: " << mnHash << std::endl << std::endl;
    uint256 uHash =  uint256(mnHash);
    uint256 uBlock;
    CTransaction tr;
    CBlockIndex* cbIndex = nullptr;

    GetTransaction(uHash, tr, uBlock, true);

    CKeyID keyID;
    walletModel->getKeyId(DecodeDestination(mnAddress), keyID);
    CPubKey pubKey;
    walletModel->getPubKey(keyID, pubKey);

    boost::optional<CKey> keyOpt;
    CKey key;
    auto addr = pubKey.GetID(); // public key ID for MN's collateral address
    if (pwalletMain->GetKey(addr, key)) // get key (private and public parts) from wallet
        keyOpt.emplace(key);

    vinKeyOpt.emplace(std::pair<CTxIn, CKey>(tr.vin[0], keyOpt.value()));

    /*for (CMasternodeConfig::CMasternodeEntry &mne : masternodeConfig.getEntries())
    {
        if( mnName == mne.getAlias())
        {
            CKey keyMasternode;
            CPubKey pubKeyMasternode;

            if(CMessageSigner::GetKeysFromSecret(mne.getPrivKey(), keyMasternode, pubKeyMasternode))
            {
                auto addr = pubKeyMasternode.GetID(); // public key ID for MN's collateral address

                if (pwalletMain->GetKey(addr, key)) // get key (private and public parts) from wallet
                {
                    //auto addr_str = EncodeDestination(key.GetPubKey().GetID());
                    keyOpt.emplace(key);
                }
                vinKeyOpt.emplace(std::pair<CTxIn, CKey>(tr.vin[0], keyOpt.value()));

                CMasternode *pmn = mnodeman.Find(pubKeyMasternode);

                if(pmn != nullptr)
                {
                    auto keyOpt = getCollateralKey(pmn);
                    if(keyOpt.is_initialized())
                    {
                        vinKeyOpt.emplace(std::pair<CTxIn, CKey>(pmn->vin, keyOpt.value()));
                        break;
                    }
                }
            }
        }
    }*/
    return vinKeyOpt;
}

std::string ValidatorWidget::createAndSendTransaction(const boost::optional<CValidatorRegister> &valRegOpt, const boost::optional<CValidatorVote> &valVoteOpt, const std::string &mnHash)
{
    std::string ret = "error";
    CCoinsView viewDummy;
    CCoinsViewCache view(&viewDummy);
    {
        LOCK(mempool.cs);
        CCoinsViewMemPool viewMempool(pcoinsTip, mempool);
        view.SetBackend(viewMempool); // temporarily switch cache backend to db+mempool view

        if (valRegOpt.is_initialized() && valVoteOpt.is_initialized())
        {
            informError("Transaction can't be for registration and for voting at the same time.");
            return ret;
        }
        else if(!valRegOpt.is_initialized() && !valVoteOpt.is_initialized())
        {
            informError("Transaction must be for registration or for voting.");
            return ret;
        }

        std::vector<CValidatorRegister> valReg;
        std::vector<CValidatorVote> valVote;

        CTxIn vin;
        if (valRegOpt.is_initialized())
        {
            //check leased to candidate coins
#ifdef ENABLE_LEASING_MANAGER
            assert(pwalletMain != NULL);
            LOCK2(cs_main, pwalletMain->cs_wallet);

            if(pwalletMain->pLeasingManager)
            {
                CAmount amount;
                CPubKey pubkey = valRegOpt.value().pubKey;
                pwalletMain->pLeasingManager->GetAllAmountsLeasedTo(pubkey, amount);

                if(amount < LEASED_TO_VALIDATOR_MIN_AMOUNT * COIN)
                {
                    ret = "Not enough leased to validator candidate coins, min=" +
                                        std::to_string(LEASED_TO_VALIDATOR_MIN_AMOUNT) +
                                        ", current=" + std::to_string(amount) + ", validator pubkey=" + HexStr(pubkey);
                    informError(ret.c_str());
                    return ret;
                }
            }
#endif
            valReg.push_back(valRegOpt.value());
            vin = valRegOpt.value().vin;
        }
        else if (valVoteOpt.is_initialized())
        {
            valVote.push_back(valVoteOpt.value());
            vin = valVoteOpt.value().vin;
        }

        //Check minimum age of mn vin
        const CCoins* unspentCoins;// = view.AccessCoins(uint256(mnHash));//view.AccessCoins(vin.prevout.hash);
        if(valRegOpt.is_initialized())
            unspentCoins = view.AccessCoins(uint256(mnHash));
        else if(valVoteOpt.is_initialized())
            unspentCoins = view.AccessCoins(vin.prevout.hash);
        int age = GetCoinsAge(unspentCoins);
        if (age < MASTERNODE_MIN_CONFIRMATIONS)
        {
            ret = "Masternode vin minimum confirmation is:" + std::to_string(MASTERNODE_MIN_CONFIRMATIONS) +
                                ", but now vin age = " + std::to_string(age);
            informError(ret.c_str());
            return ret;
        }


        // Get own address from wallet to send btcu to
        CReserveKey reservekey(pwalletMain);
        CPubKey vchPubKey;
        assert(reservekey.GetReservedKey(vchPubKey));
        CTxDestination myAddress = PKHash(vchPubKey.GetID());

        CAmount nAmount = AmountFromValue(
                UniValue((double) 38 / COIN)); // send 38 satoshi (min tx fee per kb is 100 satoshi)
        CWalletTx wtx;

        EnsureWalletIsUnlocked();
        // Create and send transaction
        SendMoney(myAddress, nAmount, wtx, false, valReg, valVote);

        // Get hash of the created transaction
        ret = wtx.GetHash().GetHex();
        return ret;
    }
}

void ValidatorWidget::showEvent(QShowEvent *event){
    /*if (mnModel) mnModel->updateMNList();
    if(!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [this]() {mnModel->updateMNList();});
    }
    timer->start(30000);*/
}

void ValidatorWidget::hideEvent(QHideEvent *event){
    //if(timer) timer->stop();
}

void ValidatorWidget::run(int type) {
    /*bool isStartMissing = type == REQUEST_START_MISSING;
    if (type == REQUEST_START_ALL || isStartMissing) {
        QString failText;
        QString inform = startAll(failText, isStartMissing) ? tr("All Masternodes started!") : failText;
        QMetaObject::invokeMethod(this, "updateModelAndInform", Qt::QueuedConnection,
                                  Q_ARG(QString, inform));
    }

    isLoading = false;*/
}

void ValidatorWidget::onError(QString error, int type) {
    /*if (type == REQUEST_START_ALL) {
        QMetaObject::invokeMethod(this, "inform", Qt::QueuedConnection,
                                  Q_ARG(QString, "Error starting all Masternodes"));
    }*/
}

void ValidatorWidget::changeTheme(bool isLightTheme, QString& theme){
    //static_cast<MNHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
}

ValidatorWidget::~ValidatorWidget()
{
    delete ui;
}
