// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/mninfodialog.h"
#include "qt/btcu/forms/ui_mninfodialog.h"
#include "walletmodel.h"
#include "wallet/wallet.h"
#include "guiutil.h"
#include "qt/btcu/qtutils.h"
#include <QDateTime>

MnInfoDialog::MnInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MnInfoDialog)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());
    setCssProperty(ui->frame, "container-border");
    ui->frame->setContentsMargins(10,10,10,10);

    //setCssProperty(ui->layoutScroll, "container-border");
    //setCssProperty(ui->scrollArea, "container-border");

    setCssProperty(ui->labelTitle, "text-title-dialog");
    QList<QWidget*> lWidjets = {ui->labelName, ui->textName, ui->labelAddress, ui->textAddress, ui->labelPubKey, ui->textPubKey, ui->labelIP, ui->textIP,
                               ui->labelTxId, ui->textTxId, ui->labelOutIndex, ui->textOutIndex, ui->labelStatus, ui->textStatus, ui->textExport};
    for(int i = 0; i < lWidjets.size(); ++i)
        setCssSubtitleScreen(lWidjets.at(i));
    setCssProperty({ui->labelDivider1, ui->labelDivider2, ui->labelDivider3, ui->labelDivider4, ui->labelDivider5, ui->labelDivider6, ui->labelDivider7, ui->labelDivider8}, "container-divider");
    setCssProperty({ui->pushCopyKey, ui->pushCopyId, ui->pushExport}, "ic-copy-big");
    setCssProperty(ui->btnEsc, "ic-close");

    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect(ui->pushCopyKey, &QPushButton::clicked, [this](){ copyInform(pubKey, "Masternode public key copied"); });
    connect(ui->pushCopyId, &QPushButton::clicked, [this](){ copyInform(txId, "Collateral tx id copied"); });
    connect(ui->pushExport, &QPushButton::clicked, [this](){ exportMN = true; accept(); });
}

void MnInfoDialog::setData(QString name, QString address, QString pubKey, QString ip, QString txId, QString outputIndex, QString status){
    this->pubKey = pubKey;
    this->txId = txId;
    QString shortPubKey = pubKey;
    QString shortTxId = txId;
    if(shortPubKey.length() > 20) {
        shortPubKey = shortPubKey.left(13) + "..." + shortPubKey.right(13);
    }
    if(shortTxId.length() > 20) {
        shortTxId = shortTxId.left(12) + "..." + shortTxId.right(12);
    }
    ui->textName->setText(name);
    ui->textAddress->setText(address);
    ui->textPubKey->setText(shortPubKey);
    ui->textIP->setText(ip);
    ui->textTxId->setText(shortTxId);
    ui->textOutIndex->setText(outputIndex);
    ui->textStatus->setText(status);
}

void MnInfoDialog::copyInform(QString& copyStr, QString message){
    GUIUtil::setClipboard(copyStr);
    if(!snackBar) snackBar = new SnackBar(nullptr, this);
    snackBar->setText(tr(message.toStdString().c_str()));
    snackBar->resize(this->width(), snackBar->height());
    openDialog(snackBar, this);
}

void MnInfoDialog::closeDialog(){
    if(snackBar && snackBar->isVisible()) snackBar->hide();
    close();
}

MnInfoDialog::~MnInfoDialog(){
    if(snackBar) delete snackBar;
    delete ui;
}
