// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2018 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REGISTERVALIDATOR_H
#define REGISTERVALIDATOR_H

#include <QDialog>
#include <QComboBox>
#include <QListView>
#include "qt/btcu/contactsdropdown.h"
#include "qt/btcu/pwidget.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "qt/btcu/addressfilterproxymodel.h"

struct masterNode
{
    std::string name;
    std::string address;
    std::string hash;
};

namespace Ui {
class RegisterValidator;
}

class RegisterValidator : public PWidget
{
    Q_OBJECT

public:
    explicit RegisterValidator( WalletModel* walletModel, PWidget *parent = nullptr);
    ~RegisterValidator();

    void registerValidator();
    void setTableModel(WalletModel* walletModel);

private Q_SLOTS:
    void onLineEditClicked(bool ownerAdd);
    void onComboBox(const QModelIndex &index);
    void onBoxClicked();
    void onRegister();

Q_SIGNALS:
    void registered(std::string MNName, std::string address, std::string hash);

private:
    void concats();
    void fillComboBox();

private:
    Ui::RegisterValidator *ui;

    QVector<masterNode> MNs;

    QAction *btnBox = nullptr;
    QListView *listViewComboBox = nullptr;
    QWidget * widgetBox = nullptr;
    AddressTableModel* addressTableModel = nullptr;
    AddressFilterProxyModel *filter = nullptr;

    bool isConcatMasternodeSelected;

    QString currentMN;
    ContactsDropdown *menuMasternodes = nullptr;
    QAction *btnMasternodeContact = nullptr;
    QAction *btnUpMasternodeContact = nullptr;
};

#endif // REGISTERVALIDATOR_H
