// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/settings/settingssignmessagewidgets.h"
#include "qt/btcu/settings/forms/ui_settingssignmessagewidgets.h"
#include "qt/btcu/qtutils.h"
#include "guiutil.h"
#include "walletmodel.h"

#include "base58.h"
#include "init.h"
#include "wallet/wallet.h"
#include "askpassphrasedialog.h"
#include "addressbookpage.h"
#include "key_io.h"

#include <string>
#include <vector>

#include <QClipboard>

SettingsSignMessageWidgets::SettingsSignMessageWidgets(BTCUGUI* _window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::SettingsSignMessageWidgets)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    ui->left->setProperty("cssClass", "container-border");
    ui->left->setContentsMargins(10,10,10,10);
    // Containers

    // Title
    ui->labelTitle->setText(tr("Sign/Verify Message"));
    ui->labelTitle->setProperty("cssClass", "text-title-screen");

    // Subtitle
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    // Address
    ui->labelSubtitleAddress->setText(tr("BTCU address or contact label"));
    //ui->labelSubtitleAddress->setProperty("cssClass", "text-title");
   setCssSubtitleScreen(ui->labelSubtitleAddress);
    ui->addressIn_SM->setPlaceholderText(tr("Enter address"));
    ui->addressIn_SM->setProperty("cssClass", "edit-primary-multi-book");
    ui->addressIn_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(ui->addressIn_SM);

    /* Button Group */
    ui->pushSign->setText(tr("Sign"));
    ui->pushVerify->setText(tr("Verify"));
    setCssProperty(ui->pushSign, "btn-check-left");
    setCssProperty(ui->pushVerify, "btn-check-left");
    ui->labelSubtitleSwitch->setText(tr("Select mode"));
    setCssProperty(ui->labelSubtitleSwitch, "text-subtitle");
    ui->pushSign->setChecked(true);
    updateMode();

    // Message
    ui->labelSubtitleMessage->setText(tr("Message"));
    //ui->labelSubtitleMessage->setProperty("cssClass", "text-title");
   setCssSubtitleScreen(ui->labelSubtitleMessage);
#if QT_VERSION >= 0x050300
    ui->messageIn_SM->setPlaceholderText(tr("Write message"));
#endif
    ui->messageIn_SM->setProperty("cssClass","edit-primary");
    setShadow(ui->messageIn_SM);
    ui->messageIn_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);

    ui->labelSubtitleSignature->setText(tr("Signature"));
    //ui->labelSubtitleSignature->setProperty("cssClass", "text-title");
   setCssSubtitleScreen(ui->labelSubtitleSignature);
    ui->signatureOut_SM->setPlaceholderText(tr("Signature"));
    ui->signatureOut_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);

    initCssEditLine(ui->signatureOut_SM);
    setShadow(ui->signatureOut_SM);

    // Buttons
    QPixmap ic("://ic-contact-arrow-down");
    btnContact = ui->addressIn_SM->addAction(QIcon(ic), QLineEdit::TrailingPosition);

    ui->pushButtonSave->setText(tr("SIGN"));
    ui->pushButtonClear->setText(tr("CLEAR ALL"));
    setCssBtnPrimary(ui->pushButtonSave);
    setCssBtnSecondary(ui->pushButtonClear);

    ui->statusLabel_SM->setStyleSheet("QLabel { color: transparent; }");

    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onGoClicked()));
    connect(btnContact, SIGNAL(triggered()), this, SLOT(onAddressesClicked()));
    connect(ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(onClearAll()));
    connect(ui->pushSign, &QPushButton::clicked, [this](){onModeSelected(true);});
    connect(ui->pushVerify,  &QPushButton::clicked, [this](){onModeSelected(false);});
}

SettingsSignMessageWidgets::~SettingsSignMessageWidgets(){
    delete ui;
}

void SettingsSignMessageWidgets::showEvent(QShowEvent *event)
{
    if (ui->addressIn_SM) ui->addressIn_SM->setFocus();
}

void SettingsSignMessageWidgets::onModeSelected(bool isSign){
    this->isSign = isSign;
    updateMode();
}

void SettingsSignMessageWidgets::onGoClicked(){
    if(isSign){
        onSignMessageButtonSMClicked();
    } else {
        onVerifyMessage();
    }
}

void SettingsSignMessageWidgets::updateMode(){
    QString subtitle;
    QString go;
    if(isSign){
        subtitle = tr("You can sign messages with your addresses to prove you own them. Be careful not to sign anything vague, as phishing attacks may try to trick you into signing your identity over to them. Only sign fully-detailed statements you agree to.");
        go = tr("SIGN");
        ui->signatureOut_SM->setReadOnly(true);
        ui->signatureOut_SM->clear();
    } else {
        subtitle = tr("Enter the signing address, message (ensure you copy line breaks, spaces, tabs, etc. exactly) and signature below to verify the message. Be careful not to read more into the signature than what is in the signed message itself, to avoid being tricked by a man-in-the-middle attack.");
        go = tr("VERIFY");
        ui->signatureOut_SM->setReadOnly(false);
    }
    ui->labelSubtitle1->setText(subtitle);
    ui->pushButtonSave->setText(go);
}

void SettingsSignMessageWidgets::setAddress_SM(const QString& address){
    ui->addressIn_SM->setText(address);
    ui->messageIn_SM->setFocus();
}

void SettingsSignMessageWidgets::onAddressBookButtonSMClicked(){
    if (walletModel && walletModel->getAddressTableModel()) {
        AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::ReceivingTab, this);
        dlg.setModel(walletModel->getAddressTableModel());
        if (dlg.exec()) {
            setAddress_SM(dlg.getReturnValue());
        }
    }
}

void SettingsSignMessageWidgets::onPasteButtonSMClicked(){
    setAddress_SM(QApplication::clipboard()->text());
}

void SettingsSignMessageWidgets::onClearAll() {
    ui->addressIn_SM->clear();
    ui->signatureOut_SM->clear();
    ui->messageIn_SM->clear();
    ui->statusLabel_SM->setStyleSheet("QLabel { color: transparent; }");
}

void SettingsSignMessageWidgets::onSignMessageButtonSMClicked(){

    if (!walletModel)
        return;
    if(walletModel->getEncryptionStatus() == WalletModel::Unencrypted){
        QMessageBox::critical(this, tr("Sign message failed"),
                              tr("Error: Wallet not encrypted. Please set up passphrase."));
        return;
    }

    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */

    ui->signatureOut_SM->clear();


    CTxDestination addr = DecodeDestination(ui->addressIn_SM->text().toStdString());
    if (!IsValidDestination(addr)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID = GetKeyForDestination(*pwalletMain, addr);
    if (keyID.IsNull()) {
        // TODO: change css..
        //ui->addressIn_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(walletModel->requestUnlock(AskPassphraseDialog::Context::Sign_Message, true));
    if (!ctx.isValid()) {
        QMessageBox::critical(this, tr("Access error"),
                              tr("Error: Please unlock wallet."));
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Private key for the entered address is not available."));
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_SM->document()->toPlainText().toStdString();

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signing failed.") + QString("</nobr>"));
        return;
    }

    ui->statusLabel_SM->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signed.") + QString("</nobr>"));

    ui->signatureOut_SM->setText(QString::fromStdString(EncodeBase64(&vchSig[0], vchSig.size())));
}

void SettingsSignMessageWidgets::onVerifyMessage(){

    /**
     * ui->addressIn_SM->clear();
    ui->signatureOut_SM->clear();
    ui->messageIn_SM->clear();
    ui->statusLabel_SM->setStyleSheet("QLabel { color: transparent; }");
     */

    CTxDestination addr = DecodeDestination(ui->addressIn_SM->text().toStdString());
    if (std::get_if<CNoDestination>(&addr)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    if (!std::get_if<PKHash>(&addr)) {
        //ui->addressIn_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(ui->signatureOut_SM->text().toStdString().c_str(), &fInvalid);

    if (fInvalid) {
        //ui->signatureOut_SM->setValid(false);
        ui->signatureOut_SM->setStyleSheet("QLabel { color: red; }");
        ui->signatureOut_SM->setText(tr("The signature could not be decoded.") + QString(" ") + tr("Please check the signature and try again."));
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_SM->document()->toPlainText().toStdString();

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(Hash(ss.begin(), ss.end()), vchSig)) {
        //ui->signatureOut_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The signature did not match the message digest.") + QString(" ") + tr("Please check the signature and try again."));
        return;
    }

    if (!(EncodeDestination(PKHash(pubkey.GetID())) == EncodeDestination(addr))) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message verification failed.") + QString("</nobr>"));
        return;
    }

    ui->statusLabel_SM->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message verified.") + QString("</nobr>"));
}

void SettingsSignMessageWidgets::onAddressesClicked(){
    int addressSize = walletModel->getAddressTableModel()->sizeRecv();
    if(addressSize == 0) {
        informError(tr("No addresses available, you can go to the receive screen and add some there!"));
        return;
    }

    //int height = (addressSize <= 2) ? ui->addressIn_SM->height() * ( 2 * (addressSize + 1 )) : ui->addressIn_SM->height() * 4;
    int height =(addressSize < 4) ? 45 * addressSize + 25 : 45 * 4 + 25;
    int width = ui->containerAddress->width()-5;

    if(!menuContacts){
        menuContacts = new ContactsDropdown(
                width,
                height,
                this
        );
       menuContacts->setGraphicsEffect(0);
        menuContacts->setWalletModel(walletModel, AddressTableModel::Receive);
        connect(menuContacts, &ContactsDropdown::contactSelected, [this](QString address, QString label){
            setAddress_SM(address);
           btnContact->setIcon(getIconComboBox(isLightTheme(),false));
        });

    }

    if(menuContacts->isVisible()){
        menuContacts->hide();
        btnContact->setIcon(getIconComboBox(isLightTheme(),false));
        return;
    }
    btnContact->setIcon(getIconComboBox(isLightTheme(),true));
    menuContacts->resizeList(width, height);
    menuContacts->setStyleSheet(this->styleSheet());
    menuContacts->adjustSize();

    //QPoint pos = ui->container_sign->mapToParent(ui->containerAddress->rect().bottomLeft());
   QPoint pos = ui->container_sign->mapToParent(ui->containerAddress->mapToParent(ui->addressIn_SM->rect().bottomLeft()));// rect().bottomLeft());
    pos.setY(pos.y() + (ui->addressIn_SM->height()  * 0.5)+ 11);
    menuContacts->move(pos);
    menuContacts->show();
}

void SettingsSignMessageWidgets::resizeMenu(){
    if(menuContacts && menuContacts->isVisible()){
        int width = ui->containerAddress->width();
        menuContacts->resizeList(width, menuContacts->height());
        menuContacts->resize(width, menuContacts->height());
       QPoint pos = ui->container_sign->mapToParent(ui->containerAddress->mapToParent(ui->addressIn_SM->rect().bottomLeft()));// rect().bottomLeft());
       pos.setY(pos.y() + (ui->addressIn_SM->height()  * 0.5)+ 11);
        menuContacts->move(pos);
    }
}

void SettingsSignMessageWidgets::resizeEvent(QResizeEvent *event){
    resizeMenu();
    QWidget::resizeEvent(event);
}
