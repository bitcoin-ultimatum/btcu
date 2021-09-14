#include "validatorrow.h"
#include "ui_validatorrow.h"
#include <iostream>

ValidatorRow::ValidatorRow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ValidatorRow)
{
    ui->setupUi(this);

    connect(ui->checkBoxVote, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged()));
}

ValidatorRow::~ValidatorRow()
{
    delete ui;
}

void ValidatorRow::updateView(QString name, QString pubKey, QString vin)
{
    ui->labelName->setText(name);
    ui->labelPublicKey->setText(pubKey);
    ui->labelVIN->setText(vin);
}

void ValidatorRow::setVoteVisible(bool isVisible)
{
    ui->checkBoxVote->setVisible(isVisible);
}

void ValidatorRow::onCheckBoxStateChanged()
{
    Q_EMIT voted(ui->labelPublicKey->text(), (ui->checkBoxVote->checkState() == Qt::Checked));
}
