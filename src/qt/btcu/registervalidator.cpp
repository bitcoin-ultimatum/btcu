// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2018 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/registervalidator.h"
#include "qt/btcu/ui_registervalidator.h"
#include "qt/btcu/qtutils.h"
#include <QRegExpValidator>
#include "../rpc/server.h"

RegisterValidator::RegisterValidator( WalletModel* walletModel, PWidget *parent) :
    PWidget(parent),
    ui(new Ui::RegisterValidator)
{
    ui->setupUi(this);

    addressTableModel = walletModel->getAddressTableModel();
    filter = new AddressFilterProxyModel(QString(AddressTableModel::Receive), this);
    filter->setSourceModel(addressTableModel);

    this->setStyleSheet(parent->styleSheet());

    ui->frame->setProperty("cssClass", "container-border");

    //Buttons
    ui->pbnBack->setProperty("cssClass", "btn-leasing-dialog-back");
    ui->pbnRegister->setProperty("cssClass", "btn-leasing-dialog-save");
    connect(ui->pbnBack, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->pbnRegister, SIGNAL(clicked()), this, SLOT(onRegister()));

    //Label
    setCssTitleScreen(ui->labelValidator);
    setCssSubtitleScreen(ui->labelMasternode);
    ui->labelValidator->setProperty("cssClass", "text-title-screen");
    ui->labelMasternode->setProperty("cssClass", "text-leasing-dialog");

    //Line edit
    /*ui->lineEditMasternode->setPlaceholderText(tr("Masternode Name"));
    setShadow(ui->lineEditMasternode);
    //ui->lineEditMasternode->setProperty("cssClass", "edit-leasing-dialog");
    btnMasternodeContact = ui->lineEditMasternode->addAction(getIconComboBox(isLightTheme(), false), QLineEdit::TrailingPosition);
    setCssProperty(ui->lineEditMasternode, "edit-primary-multi-book");
    ui->lineEditMasternode->setAttribute(Qt::WA_MacShowFocusRect, 0);
    connect(btnMasternodeContact, &QAction::triggered, [this](){ onLineEditClicked(true); });
    //ui->lineEditBTCU->setPlaceholderText(tr("0"));
    //ui->lineEditBTCU->setProperty("cssClass","edit-primary-BTCU");
    btnUpMasternodeContact = ui->lineEditMasternode->addAction(getIconComboBox(isLightTheme(),true), QLineEdit::TrailingPosition);
    connect(btnUpMasternodeContact, &QAction::triggered, [this](){ onLineEditClicked(true); });
    ui->lineEditMasternode->removeAction(btnUpMasternodeContact);*/

    //ComboBox
    setCssProperty(ui->lineEditMNName, "edit-primary-multi-book");
    btnBox = ui->lineEditMNName->addAction(getIconComboBox(isLightTheme(),false), QLineEdit::TrailingPosition);
    connect(btnBox, &QAction::triggered, [this](){ onBoxClicked(); });
    SortEdit* lineEdit = new SortEdit(ui->comboBox);
    initComboBox(ui->comboBox, lineEdit);
    connect(lineEdit, &SortEdit::Mouse_Pressed, [this](){ui->comboBox->showPopup();});
    ui->lineEditMNName->setReadOnly(true);

    fillComboBox();

    ui->comboBox->setVisible(false);
    ui->lineEditMNName->setText(ui->comboBox->currentText());
    widgetBox = new QWidget(this);
    QVBoxLayout* LayoutBox = new QVBoxLayout(widgetBox);
    listViewComboBox = new QListView();
    LayoutBox->addWidget(listViewComboBox);
    widgetBox->setGraphicsEffect(0);
    listViewComboBox->setProperty("cssClass", "container-border-light");
    listViewComboBox->setModel(ui->comboBox->model());
    connect(listViewComboBox, SIGNAL(clicked(QModelIndex)), this, SLOT(onComboBox(QModelIndex)));
    widgetBox->hide();
}

RegisterValidator::~RegisterValidator()
{
    delete ui;
}

void RegisterValidator::setTableModel(WalletModel *walletModel)
{
    addressTableModel = walletModel->getAddressTableModel();
    filter = new AddressFilterProxyModel(QString(AddressTableModel::Receive), this);
    filter->setSourceModel(addressTableModel);
}

void RegisterValidator::registerValidator(){
    std::string name = "";
    std::string hash = "";

    std::string strConfFile = "masternode.conf";
    std::string strDataDir = GetDataDir().string();
    if (strConfFile != boost::filesystem::basename(strConfFile) + boost::filesystem::extension(strConfFile)){
        //throw std::runtime_error(strprintf(_("masternode.conf %s resides outside data directory %s"), strConfFile, strDataDir));
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

                std::string buffLine = "";
                int count = 0;
                for (int i = 0; i < line.size(); ++i) {
                    if (line.at(i) == ' ') {
                        if (count == 0) {
                            name = buffLine;
                        } else if (count == 3) {
                            hash = buffLine;
                        }
                        buffLine = "";
                        ++count;
                    } else {
                        buffLine += line.at(i);
                    }
                }

                uint256 uHash = uint256(hash);
                uint256 uBlock;
                CTransaction tr;
                int blockHeight = -1;
                CBlockIndex *cbIndex = nullptr;

                //GetTransaction(uHash, tr, uBlock, true);
                //IsTransactionInChain(uHash, blockHeight, tr);

                /*CKeyID key;
                walletModel->getKeyId(EncodeDestination(walletModel->getAddressTableModel()->getAddressToShow().toStdString()), key);
                CPubKey pubKey;
                walletModel->getPubKey(key, pubKey);

                QString type = "-";
                UniValue uniValue;
                uniValue = mnvalidatorlist(uniValue, true);
                std::string keys = uniValue.get_str();
                //if(CPubKey(keys) == pubKey)
                //    type = "Validator";
                //else
                type = "Master";
                CTxOut out;
                CAmount leasingAmount;
                //pleasingManagerMain->GetAllAmountsLeasedTo(pubKey, leasingAmount);

                for(auto i : tr.GetOutPoints())
                {
                    //out = pleasingManagerMain->CalcLeasingReward(i, pubKey.GetID());
                    //LeaserType t = static_cast<LeaserType>(i.n);
                    //if(t == LeaserType::MasterNode)
                    //     type =  "Masternode";
                    //else if(t == LeaserType::ValidatorNode)
                    //    type = "Validator";
                    //else
                    //    type = "-";
                }*/
            }
        }
    }
}

void RegisterValidator::onBoxClicked() {
    if(widgetBox->isVisible()){
        widgetBox->hide();
        btnBox->setIcon(getIconComboBox(isLightTheme(),false));
        return;
    }
    btnBox->setIcon(getIconComboBox(isLightTheme(),true));
    QPoint pos = ui->lineEditMNName->pos();
    QPoint point = ui->lineEditMNName->rect().bottomRight();

    widgetBox->setFixedSize(ui->lineEditMNName->width() + 20,(ui->comboBox->count()*50));
    pos.setY(window->height()/2 + point.y());
    pos.setX(window->width()/2 - point.x()/2 - 13);
    widgetBox->move(pos);
    widgetBox->show();
}

void RegisterValidator::onComboBox(const QModelIndex &index) {
    QString value = index.data(0).toString();
    ui->lineEditMNName->setText(value);
    ui->comboBox->setCurrentIndex(index.row());
    widgetBox->hide();
}

void RegisterValidator::onLineEditClicked(bool ownerAdd) {
    isConcatMasternodeSelected = ownerAdd;
    concats();
}

void RegisterValidator::fillComboBox()
{
    ui->comboBox->addItem("Masternode name");

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
                std::string hash = "";
                std::string address = "";

                std::string buffLine = "";
                int count = 0;
                for (int i = 0; i < line.size(); ++i) {
                    if (line.at(i) == ' ')
                    {
                        if(count == 3) break;
                        count++;
                    }
                    else if(count == 0) name += line.at(i);
                    else if(count == 3) hash += line.at(i);
                }

                int rowCount = filter->rowCount();
                for(int addressNumber = 0; addressNumber < rowCount; addressNumber++)
                {
                    QModelIndex rowIndex = filter->index(addressNumber, AddressTableModel::Address);
                    QModelIndex sibling = rowIndex.sibling(addressNumber, AddressTableModel::Label);
                    QString label = sibling.data(Qt::DisplayRole).toString();
                    if(label.toStdString() == name)
                    {
                        sibling = rowIndex.sibling(addressNumber, AddressTableModel::Address);
                        address = sibling.data(Qt::DisplayRole).toString().toStdString();
                        break;
                    }
                }

                if(address == "") continue;
                MNs.push_back({name, address, hash});
                QString text = QString::fromStdString(name) + ":" + QString::fromStdString(address);

                if(text.length() > 40)
                    text = text.left(37) + "...";

                ui->comboBox->addItem(text);
            }
        }
    }
}

void RegisterValidator::concats()
{
    /*
    int masternodeSize = walletModel->getAddressTableModel()->sizeRecv();

    int height;
    int width;
    QPoint pos;

    height = ui->lineEditMasternode->height();
    width = ui->lineEditMasternode->width();
    pos = ui->lineEditMasternode->rect().bottomLeft();
    pos.setY((pos.y() + (height +4) * 2.4));

    pos.setX(pos.x() + 30);
    height = 45;
    height = (masternodeSize < 4) ? height * masternodeSize + 25 : height * 4 + 25;

    if(!menuMasternodes)
    {
        menuMasternodes = new ContactsDropdown(width, height, this);

        menuMasternodes->setGraphicsEffect(0);

        connect(menuMasternodes, &ContactsDropdown::contactSelected, [this](QString address, QString label){ //???
            if (isConcatMasternodeSelected) {
                ui->lineEditMasternode->setText(label);
                currentMN = address;
            } else {
                //SendMultiRow
            }
            ui->lineEditMasternode->removeAction(btnUpMasternodeContact);
            ui->lineEditMasternode->addAction(btnMasternodeContact, QLineEdit::TrailingPosition);
        });
    }
    
    if(menuMasternodes->isVisible()){
        menuMasternodes->hide();
        ui->lineEditMasternode->removeAction(btnUpMasternodeContact);
        ui->lineEditMasternode->addAction(btnMasternodeContact, QLineEdit::TrailingPosition);
        return;
    }

    ui->lineEditMasternode->removeAction(btnMasternodeContact);
    ui->lineEditMasternode->addAction(btnUpMasternodeContact, QLineEdit::TrailingPosition);
    menuMasternodes->setWalletModel(walletModel, isConcatMasternodeSelected ? AddressTableModel::Receive : AddressTableModel::LeasingSend);
    menuMasternodes->resizeList(width, height);
    menuMasternodes->setStyleSheet(styleSheet());
    menuMasternodes->adjustSize();
    menuMasternodes->move(pos);
    menuMasternodes->show();
     */
}

void RegisterValidator::onRegister()
{
    if(ui->comboBox->currentIndex() == 0)
    {
        setCssEditLine(ui->lineEditMNName, false, true);
        return;
    }

    /*QString name = "";
    for(int i = 0 ; i < ui->comboBox->currentText().size(); ++i)
    {
        if(ui->comboBox->currentText().at(i) == ":")
            break;
        name += ui->comboBox->currentText().at(i);
    }*/
    this->hide();
    masterNode mn = MNs[ui->comboBox->currentIndex()-1];
    Q_EMIT registered(mn.name, mn.address, mn.hash);
    this->close();
}
