// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2018 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATORWIDGET_H
#define VALIDATORWIDGET_H

#include <QWidget>
#include "qt/btcu/pwidget.h"
#include "qt/btcu/furabstractlistitemdelegate.h"
#include "qt/btcu/validatorrow.h"
#include "qt/btcu/mnmodel.h"
#include "qt/btcu/tooltipmenu.h"
#include <QTimer>
#include <atomic>
#include <QSpacerItem>


class BTCUGUI;

namespace Ui {
class ValidatorWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

struct Validators{
    std::string name;
    std::string pubKey;
    CTxIn vin;
    bool vote;
};

class ValidatorWidget : public PWidget
{
    Q_OBJECT

public:

    explicit ValidatorWidget(BTCUGUI *parent = nullptr);
    ~ValidatorWidget();

    void loadWalletModel() override;

    void run(int type) override;
    void onError(QString error, int type) override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void showRowsOfValidators(bool isVote);
    void savingNewValidator(std::string pubKey, CTxIn vin, bool isVote);
    void savingActiveValidator(std::string pubKey, CTxIn vin);
    std::string createAndSendTransaction(const boost::optional<CValidatorRegister> &valRegOpt, const boost::optional<CValidatorVote> &valVoteOpt, const std::string &mnHash);
    boost::optional<CValidatorRegister> createValidatorReg(const std::string &mnName, const std::string &mnHash, const std::string &mnAddress);
    boost::optional<std::pair<CTxIn, CKey>> getVinKey(const std::string &mnName, const std::string &mnHash, const std::string &mnAddress);
    boost::optional<CKey> getCollateralKey(CMasternode *pmn);
    boost::optional<CValidatorVote> createValidatorVote(const std::vector<MNVote> &votes);
    boost::optional<std::pair<CTxIn, CKey>> getRegisteringValidatorVinKey();

Q_SIGNALS:

   void CreateValidator();

private Q_SLOTS:
    void changeTheme(bool isLightTheme, QString &theme) override;

   void onpbnValidatorClicked();
   void onpbnRegisteredClicked();
   void onpbnVoteClicked();
   void onpbnActiveClicked();
   void onPushButtonVote();
   void onRegisterValidator(std::string MNName, std::string address, std::string hash);
   void onVoted(QString pubKey, bool vote);

private:
    Ui::ValidatorWidget *ui;
    FurAbstractListItemDelegate *delegate;
    TooltipMenu* menu = nullptr;
    QModelIndex index;
    QTimer *timer = nullptr;

    QVector<std::shared_ptr<ValidatorRow>> registeredValidatorsRowVector;
    QVector<std::shared_ptr<ValidatorRow>> activeValidatorsRowVector;
    QVector<ValidatorRow*> listVector;
    QVector<ValidatorRow*> voteVector;
    QVector<Validators> registeredValidators;
    QVector<Validators> activeValidators;
   std::atomic<bool> isLoading;
   bool bShowHistory = false;
   bool bShowHistoryMy = false;
   void showHistory();
   int n = 0;
   QSpacerItem* SpacerNode = nullptr;
   QSpacerItem* SpacerNodeMy = nullptr;

};

#endif // VALIDATORWIDGET_H
