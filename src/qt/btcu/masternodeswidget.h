// Copyright (c) 2019-2020 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODESWIDGET_H
#define MASTERNODESWIDGET_H

#include <QWidget>
#include "qt/btcu/pwidget.h"
#include "qt/btcu/furabstractlistitemdelegate.h"
#include "qt/btcu/addressfilterproxymodel.h"
#include "qt/btcu/mnrow.h"
#include "qt/btcu/mnmodel.h"
#include "qt/btcu/tooltipmenu.h"
#include "addresstablemodel.h"
#include <QTimer>
#include <atomic>
#include <QSpacerItem>
#include <QSharedPointer>

#define CREATE_MN_AMOUNT 1000

class BTCUGUI;

namespace Ui {
class MasterNodesWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class MasterNodesWidget : public PWidget
{
    Q_OBJECT

public:
    explicit MasterNodesWidget(BTCUGUI *parent = nullptr);
    ~MasterNodesWidget();

    void loadWalletModel() override;

    void run(int type) override;
    void onError(QString error, int type) override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void clearScrollWidget();

Q_SIGNALS:
   void CreateMasternode();

private Q_SLOTS:
    void onCreateMNClicked();
    void onStartAllClicked(int type);
    void changeTheme(bool isLightTheme, QString &theme) override;
    void onUpgradeMNClicked();
    void onDeleteMNClicked();
    void onInfoMNClicked();
    void updateListState();
    void updateModelAndInform(QString informText);
    void onpbnMenuClicked(QModelIndex index);

   void onTempADD();
   void onpbnMasternodeClicked();
   void onpbnValidatorClicked();
   void onpbnMyMasternodesClicked();
   void onpbnGlobalMasternodesClicked();

private:
    Ui::MasterNodesWidget *ui;
    FurAbstractListItemDelegate *delegate;
    AddressTableModel* addressTableModel = nullptr;
    AddressFilterProxyModel *filter = nullptr;
    MNModel *mnModel = nullptr;
    TooltipMenu* menu = nullptr;
   TooltipMenu* menuMy = nullptr;
    QModelIndex index;
    QTimer *timer = nullptr;

    QVector<QSharedPointer<MNRow>> MNRows;
    std::atomic<bool> isLoading;

    bool checkMNsNetwork();
    void startAlias(QString strAlias);
    bool startAll(QString& failedMN, bool onlyMissing);
    bool startMN(CMasternodeConfig::CMasternodeEntry mne, std::string& strError);
    void removeMNLine();
// temp
   bool bShowHistory = false;
   bool bShowHistoryMy = false;
   void showHistory();
   int n = 0;
   QSpacerItem* SpacerNode = nullptr;
   QSpacerItem* SpacerNodeMy = nullptr;
   //
};

#endif // MASTERNODESWIDGET_H
