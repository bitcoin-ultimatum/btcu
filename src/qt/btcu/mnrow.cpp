// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/mnrow.h"
#include "qt/btcu/forms/ui_mnrow.h"
#include "qt/btcu/qtutils.h"

MNRow::MNRow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MNRow)
{
    ui->setupUi(this);

   setCssProperty(ui->Contener, "container-border");
   setCssSubtitleScreen(ui->labelName);
   ui->labelAddress->setProperty("cssClass","text-list-amount-send");
   setCssSubtitleScreen(ui->labelLeasing);
   setCssSubtitleScreen(ui->labelBlockHeight);
   setCssSubtitleScreen(ui->labelType);
   setCssSubtitleScreen(ui->labelProfit);
   connect(ui->pushButtonMenu, SIGNAL(clicked()), this, SLOT(onPbnMenuClicked()));

}

void MNRow::updateView(std::string name, std::string address, double leasing, int blockHeight, QString type, double profit){
    ui->labelName->setText(QString::fromStdString(name));
    ui->labelAddress->setText(QString::fromStdString(address));
    ui->labelLeasing->setText(QString::number(leasing, 'g', 12));
    ui->labelBlockHeight->setText(blockHeight != -1 ? QString::number(blockHeight) : "-");
    ui->labelType->setText(type);
    ui->labelProfit->setText(profit > 0 ? QString::number(profit, 'g', 12) : "-");
}

void MNRow::setIndex(QModelIndex index)
{
    this->index = index;
}

QModelIndex MNRow::getIndex()
{
    return this->index;
}

void MNRow::onPbnMenuClicked()
{
   Q_EMIT onMenuClicked(index);
}

MNRow::~MNRow(){
    delete ui;
}
