// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/masternodewizarddialog.h"
#include "qt/btcu/forms/ui_masternodewizarddialog.h"
#include "qt/btcu/qtutils.h"
#include "optionsmodel.h"
#include "pairresult.h"
#include "activemasternode.h"
#include "qt/btcu/guitransactionsutils.h"
#include "key_io.h"
#include <QFile>
#include <QIntValidator>
#include <QHostAddress>
#include <QRegExp>
#include <QRegExpValidator>

MasterNodeWizardDialog::MasterNodeWizardDialog(WalletModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MasterNodeWizardDialog),
    icConfirm1(new QPushButton()),
    icConfirm3(new QPushButton()),
    icConfirm4(new QPushButton()),
    walletModel(model)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    setCssProperty(ui->frame, "container-border");
    ui->frame->setContentsMargins(10,10,10,10);

    setCssProperty({ui->labelLine1, ui->labelLine3}, "line-purple");
    setCssProperty({ui->groupBoxName, ui->groupContainer}, "container-border");
    setCssProperty({ui->pushNumber1, ui->pushNumber3, ui->pushNumber4}, "btn-number-check");
    setCssProperty({ui->pushName1, ui->pushName3, ui->pushName4}, "btn-name-check");

    ui->pushNumber1->setEnabled(false);
    ui->pushNumber3->setEnabled(false);
    ui->pushNumber4->setEnabled(false);
    ui->pushName1->setEnabled(false);
    ui->pushName3->setEnabled(false);
    ui->pushName4->setEnabled(false);

    // Frame 1
    setCssProperty(ui->labelTitle1, "text-title-dialog");
    setCssProperty(ui->labelMessage1a, "text-main-grey");
    setCssProperty(ui->labelMessage1b, "text-main-purple");

    // Frame 3
    setCssProperty(ui->labelTitle3, "text-title-dialog");
    setCssProperty(ui->labelMessage3, "text-main-grey");

    ui->lineEditName->setPlaceholderText(tr("e.g user_masternode"));
    ui->lineEditName->setProperty("cssClass", "edit-leasing-dialog");
    //setCssProperty(ui->lineEditName, "edit-primary-multi-book");
    //initCssEditLine(ui->lineEditName);
    ui->lineEditName->setValidator(new QRegExpValidator(QRegExp("^[A-Za-z0-9]+"), ui->lineEditName));

    // Frame 4
    setCssProperty(ui->labelTitle4, "text-title-dialog");
    setCssProperty({ui->labelSubtitleIp, ui->labelSubtitlePort}, "text-main-purple");
    //setCssSubtitleScreen(ui->labelSubtitleAddressIp);
    setCssProperty(ui->labelSubtitleAddressIp, "text-main-grey");

    ui->lineEditIpAddress->setPlaceholderText("e.g 18.255.255.255");
    ui->lineEditPort->setPlaceholderText("e.g 3666");
    //setCssProperty(ui->lineEditIpAddress, "edit-primary-multi-book");
    //setCssProperty(ui->lineEditPort, "edit-primary-multi-book");

    ui->lineEditIpAddress->setProperty("cssClass", "edit-leasing-dialog");
    ui->lineEditPort->setProperty("cssClass", "edit-leasing-dialog");
    //initCssEditLine(ui->lineEditIpAddress);
    //initCssEditLine(ui->lineEditPort);
    ui->stackedWidget->setCurrentIndex(pos);
    ui->lineEditPort->setValidator(new QIntValidator(0, 9999999, ui->lineEditPort));
    if(walletModel->isTestNetwork()){
        ui->lineEditPort->setEnabled(false);
        ui->lineEditPort->setText("13666");
    } else {
        ui->lineEditPort->setText("3666");
    }

    // Confirm icons
    ui->stackedIcon1->addWidget(icConfirm1);
    ui->stackedIcon3->addWidget(icConfirm3);
    ui->stackedIcon4->addWidget(icConfirm4);
    initBtn({icConfirm1, icConfirm3, icConfirm4});
    setCssProperty({icConfirm1, icConfirm3, icConfirm4}, "ic-step-confirm");

    // Connect btns
    setCssBtnSecondary(ui->btnNext);
    setCssBtnPrimary(ui->btnBack);
    ui->btnNext->setText(tr("NEXT"));
    ui->btnBack->setVisible(false);
    ui->btnBack->setText(tr("BACK"));

    connect(ui->pushButtonSkip, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(onNextClicked()));
    connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(onBackClicked()));
}

void MasterNodeWizardDialog::showEvent(QShowEvent *event)
{
    //if (ui->btnNext) ui->btnNext->setFocus();
}

void MasterNodeWizardDialog::onNextClicked(){
    switch(pos){
        case 0:{
            ui->stackedWidget->setCurrentIndex(1);
            ui->pushName4->setChecked(false);
            ui->pushName3->setChecked(true);
            ui->pushName1->setChecked(true);
            icConfirm1->setVisible(true);
            ui->pushNumber3->setChecked(true);
            ui->btnBack->setVisible(true);
            ui->lineEditName->setFocus();
            break;
        }
        case 1:{
            QString name = ui->lineEditName->text();
            // No empty names accepted.
            if (name.isEmpty()) {
                setCssEditLine(ui->lineEditName, false, true);
                inform("Enter masternode's name");
                return;
            }
            if(name.size() > 255)
            {
                setCssEditLine(ui->lineEditName, false, true);
                inform("Maximum length of the name is 255 characters>");
                return;
            }
            QRegExp expr("[a-zA-Z0-9]");
            for(auto ch : name)
            {
                if(!expr.exactMatch(ch))
                {
                    setCssEditLine(ui->lineEditName, false, true);
                    inform("Name must contain only digits and letters");
                    return;
                }
            }
            if(checkName(name))
            {
                setCssEditLine(ui->lineEditName, false, true);
                inform("Masternode with the same name already exists");
                return;
            }
            setCssEditLine(ui->lineEditName, true, true);

            ui->stackedWidget->setCurrentIndex(2);
            ui->pushName4->setChecked(false);
            ui->pushName3->setChecked(true);
            ui->pushName1->setChecked(true);
            icConfirm3->setVisible(true);
            ui->pushNumber4->setChecked(true);
            ui->btnBack->setVisible(true);
            ui->lineEditIpAddress->setFocus();
            break;
        }
        case 2:{

            QString address = ui->lineEditIpAddress->text();
            if (address.isEmpty()) {
                setCssEditLine(ui->lineEditIpAddress, false, true);
                inform("Enter IP address");
                return;
            }
            QHostAddress hostAddress(address);
            QAbstractSocket::NetworkLayerProtocol layerProtocol = hostAddress.protocol();
            if(layerProtocol == -1)
            {
                setCssEditLine(ui->lineEditIpAddress, false, true);
                inform("Enter correct IPv4 or IPv6 address format");
                return;
            }
            if (ui->lineEditPort->text().toInt() <= 0 || ui->lineEditPort->text().toInt() > 999999){
                setCssEditLine(ui->lineEditPort, false, true);
                inform("Invalid port number");
                return;
            }

            icConfirm4->setVisible(true);
            ui->btnBack->setVisible(true);
            ui->btnBack->setVisible(true);
            isOk = createMN();
            if(!isOk) {
                inform(returnStr);
                return;
            }
            accept();
        }
    }
    pos++;
}

bool MasterNodeWizardDialog::checkName(QString newName)
{
    newName = newName.trimmed();
    std::string strConfFile = "masternode.conf";
    std::string strDataDir = GetDataDir().string();
    if (strConfFile != boost::filesystem::basename(strConfFile) + boost::filesystem::extension(strConfFile)){
        throw std::runtime_error(strprintf(_("masternode.conf %s resides outside data directory %s"), strConfFile, strDataDir));
    }

    boost::filesystem::path pathBootstrap = GetDataDir() / strConfFile;
    if (boost::filesystem::exists(pathBootstrap)) {
        boost::filesystem::path pathMasternodeConfigFile = GetMasternodeConfigFile();
        boost::filesystem::ifstream streamConfig(pathMasternodeConfigFile);

        if (streamConfig.good()) {

            int linenumber = 1;
            for (std::string line; std::getline(streamConfig, line); linenumber++) {
                if (line.empty()) continue;
                if (line.at(0) == '#') continue;

                std::string name = "";

                int count = 0;
                for (int i = 0; i < line.size(); ++i) {
                    if (line.at(i) == ' ') {
                        if (count == 3) break;
                        count++;
                    }
                    else if (count == 0) name += line.at(i);
                }
                if(name == newName.toStdString()) return true;
            }
        }
    }
    return false;
}

bool MasterNodeWizardDialog::createMN(){
    if (walletModel) {
        /**
         *
        1) generate the mn key.
        2) create the mn address.
        3) send a tx with 10k to that address.
        4) get thereate output.
        5) use those values on the masternode.conf
         */

        // First create the mn key
        CKey secret;
        secret.MakeNewKey(false);
        std::string mnKeyString = EncodeSecret(secret);

        // second create mn address
        QString addressLabel = ui->lineEditName->text().trimmed();

        std::string alias = addressLabel.toStdString();

        QString addressStr = ui->lineEditIpAddress->text();
        QString portStr = ui->lineEditPort->text();
        if (addressStr.isEmpty() || portStr.isEmpty()) {
            returnStr = tr("IP or port cannot be empty");
            return false;
        }
        // TODO: Validate IP address..
        int portInt = portStr.toInt();
        if (portInt <= 0 && portInt > 999999){
            returnStr = tr("Invalid port number");
            return false;
        }
        // ip + port
        std::string ipAddress = addressStr.toStdString();
        std::string port = portStr.toStdString();

        // New receive address
        CTxDestination address;
        PairResult r = walletModel->getNewAddress(address, alias);

        if (!r.result) {
            // generate address fail
            inform(tr(r.status->c_str()));
            return false;
        }

        // const QString& addr, const QString& label, const CAmount& amount, const QString& message
        SendCoinsRecipient sendCoinsRecipient(QString::fromStdString(EncodeDestination(address)), QString::fromStdString(alias), CAmount(MN_DEPOSIT_SIZE) * COIN, "");

        // Send the 10 tx to one of your address
        QList<SendCoinsRecipient> recipients;
        recipients.append(sendCoinsRecipient);
        WalletModelTransaction currentTransaction(recipients);
        WalletModel::SendCoinsReturn prepareStatus;

        prepareStatus = walletModel->prepareTransaction(currentTransaction);

        QString returnMsg = "Unknown error";
        // process prepareStatus and on error generate message shown to user
        CClientUIInterface::MessageBoxFlags informType;
        returnMsg = GuiTransactionsUtils::ProcessSendCoinsReturn(
                this,
                prepareStatus,
                walletModel,
                informType, // this flag is not needed
                BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(),
                                             currentTransaction.getTransactionFee()),
                true
        );

        if (prepareStatus.status != WalletModel::OK) {
            returnStr = tr("Prepare master node failed.\n\n%1\n").arg(returnMsg);
            return false;
        }

        WalletModel::SendCoinsReturn sendStatus = walletModel->sendCoins(currentTransaction);
        // process sendStatus and on error generate message shown to user
        returnMsg = GuiTransactionsUtils::ProcessSendCoinsReturn(
                this,
                sendStatus,
                walletModel,
                informType
        );

        if (sendStatus.status == WalletModel::OK) {
            // now change the conf
            std::string strConfFile = "masternode.conf";
            std::string strDataDir = GetDataDir().string();
            if (strConfFile != boost::filesystem::basename(strConfFile) + boost::filesystem::extension(strConfFile)){
                throw std::runtime_error(strprintf(_("masternode.conf %s resides outside data directory %s"), strConfFile, strDataDir));
            }

            boost::filesystem::path pathBootstrap = GetDataDir() / strConfFile;
            if (boost::filesystem::exists(pathBootstrap)) {
                boost::filesystem::path pathMasternodeConfigFile = GetMasternodeConfigFile();
                boost::filesystem::ifstream streamConfig(pathMasternodeConfigFile);

                if (!streamConfig.good()) {
                    returnStr = tr("Invalid masternode.conf file");
                    return false;
                }

                int linenumber = 1;
                std::string lineCopy = "";
                for (std::string line; std::getline(streamConfig, line); linenumber++) {
                    if (line.empty()) continue;

                    std::istringstream iss(line);
                    std::string comment, alias, ip, privKey, txHash, outputIndex;

                    if (iss >> comment) {
                        if (comment.at(0) == '#') continue;
                        iss.str(line);
                        iss.clear();
                    }

                    if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                        iss.str(line);
                        iss.clear();
                        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                            streamConfig.close();
                            returnStr = tr("Error parsing masternode.conf file");
                            return false;
                        }
                    }
                    lineCopy += line + "\n";
                }

                if (lineCopy.size() == 0) {
                    lineCopy = "# Masternode config file\n"
                               "# Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index\n"
                               "# Example: mn1 127.0.0.2:3666 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0"
                               "#";
                }
                lineCopy += "\n";

                streamConfig.close();

                CWalletTx* walletTx = currentTransaction.getTransaction();
                std::string txID = walletTx->GetHash().GetHex();
                int indexOut = -1;
                for (int i=0; i < (int)walletTx->vout.size(); i++){
                    CTxOut& out = walletTx->vout[i];
                    if (out.nValue == MN_DEPOSIT_SIZE * COIN){
                        indexOut = i;
                    }
                }
                if (indexOut == -1) {
                    returnStr = tr("Invalid collateral output index");
                    return false;
                }
                std::string indexOutStr = std::to_string(indexOut);

                // Check IP address type
                QHostAddress hostAddress(addressStr);
                QAbstractSocket::NetworkLayerProtocol layerProtocol = hostAddress.protocol();
                if (layerProtocol == QAbstractSocket::IPv6Protocol) {
                    ipAddress = "["+ipAddress+"]";
                }

                boost::filesystem::path pathConfigFile("masternode_temp.conf");
                if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir() / pathConfigFile;
                FILE* configFile = fopen(pathConfigFile.string().c_str(), "w");
                lineCopy += alias+" "+ipAddress+":"+port+" "+mnKeyString+" "+txID+" "+indexOutStr+"\n";
                fwrite(lineCopy.c_str(), std::strlen(lineCopy.c_str()), 1, configFile);
                fclose(configFile);

                boost::filesystem::path pathOldConfFile("old_masternode.conf");
                if (!pathOldConfFile.is_complete()) pathOldConfFile = GetDataDir() / pathOldConfFile;
                if (boost::filesystem::exists(pathOldConfFile)) {
                    boost::filesystem::remove(pathOldConfFile);
                }
                rename(pathMasternodeConfigFile, pathOldConfFile);

                boost::filesystem::path pathNewConfFile("masternode.conf");
                if (!pathNewConfFile.is_complete()) pathNewConfFile = GetDataDir() / pathNewConfFile;
                rename(pathConfigFile, pathNewConfFile);

                mnEntry = masternodeConfig.add(alias, ipAddress+":"+port, mnKeyString, txID, indexOutStr);

                returnStr = tr("Master node created!");
                return true;
            } else{
                returnStr = tr("masternode.conf file doesn't exists");
            }
        } else {
            returnStr = tr("Cannot send collateral transaction.\n\n%1").arg(returnMsg);
        }
    }
    return false;
}

void MasterNodeWizardDialog::onBackClicked(){
    if (pos == 0) return;
    pos--;
    switch(pos){
        case 0:{
            ui->stackedWidget->setCurrentIndex(0);
            //ui->btnNext->setFocus();
            ui->pushNumber1->setChecked(true);
            ui->pushNumber4->setChecked(false);
            ui->pushNumber3->setChecked(false);
            ui->pushName4->setChecked(false);
            ui->pushName3->setChecked(false);
            ui->pushName1->setChecked(true);
            icConfirm1->setVisible(false);
            ui->btnBack->setVisible(false);
            break;
        }
        case 1:{
            ui->stackedWidget->setCurrentIndex(1);
            ui->lineEditName->setFocus();
            ui->pushNumber4->setChecked(false);
            ui->pushNumber3->setChecked(true);
            ui->pushName4->setChecked(false);
            ui->pushName3->setChecked(true);
            icConfirm3->setVisible(false);

            break;
        }
    }
}

void MasterNodeWizardDialog::inform(QString text){
    if (!snackBar)
        snackBar = new SnackBar(nullptr, this);
    snackBar->setText(text);
    snackBar->resize(this->width(), snackBar->height());
    openDialog(snackBar, this);
}

QSize BUTTON_SIZE = QSize(22, 22);
void MasterNodeWizardDialog::initBtn(std::initializer_list<QPushButton*> args){
    for (QPushButton* btn : args) {
        btn->setMinimumSize(BUTTON_SIZE);
        btn->setMaximumSize(BUTTON_SIZE);
        btn->move(0, 0);
        btn->show();
        btn->raise();
        btn->setVisible(false);
    }
}

MasterNodeWizardDialog::~MasterNodeWizardDialog(){
    if(snackBar) delete snackBar;
    delete ui;
}
