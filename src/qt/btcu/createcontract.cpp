#include "createcontract.h"
#include "ui_createcontract.h"
#include "qt/btcu/qtutils.h"
#include "../src/validation.h"
#include "../src/utilmoneystr.h"
//#include "../src/utilstrencodings.h"
//#include "contract.h"
#include <univalue.h>
#include <qtum/qtumtransaction.h>
#include "core_io.h"
#include "key_io.h"

CreateContract::CreateContract(BTCUGUI *parent) :
    PWidget(parent),
    ui(new Ui::CreateContract)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());

    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /*Title*/
    setCssTitleScreen(ui->labelTitle);
    ui->labelTitle->setFont(fontLight);
    setCssTitleScreen(ui->labelParameters);
    ui->labelParameters->setFont(fontLight);

    setCssProperty(ui->labelBytecode, "text-body2-grey");
    setCssProperty(ui->labelGasLimit, "text-body2-grey");
    setCssProperty(ui->labelGasPrice, "text-body2-grey");
    setCssProperty(ui->labelSenderAddress, "text-body2-grey");

    /*TextEdit*/
    ui->textEditBytecode->setProperty("cssClass", "edit-primary-multi-book");
    ui->textEditBytecode->setPlaceholderText(tr("Enter bytecode"));

    /*LineEdit*/
    ui->lineEditSenderAddress->setPlaceholderText(tr("Enter address"));
    setShadow(ui->lineEditSenderAddress);
    ui->lineEditSenderAddress->setProperty("cssClass", "edit-primary-multi-book");
    btnAddressContact = ui->lineEditSenderAddress->addAction(getIconComboBox(isLightTheme(), false), QLineEdit::TrailingPosition);
    ui->lineEditSenderAddress->setAttribute(Qt::WA_MacShowFocusRect, 0);
    connect(btnAddressContact, &QAction::triggered, [this](){ onLineEditClicked(); });
    btnUpAddressContact = ui->lineEditSenderAddress->addAction(getIconComboBox(isLightTheme(),true), QLineEdit::TrailingPosition);
    connect(btnUpAddressContact, &QAction::triggered, [this](){ onLineEditClicked(); });
    ui->lineEditSenderAddress->removeAction(btnUpAddressContact);

    ui->lineEditGasLimit->setProperty("cssClass", "edit-primary-multi-book");
    ui->lineEditGasLimit->setValidator(new QRegExpValidator(QRegExp("[0-9]+"), ui->lineEditGasLimit));
    ui->lineEditGasLimit->setPlaceholderText(tr("Enter gas limit"));
    ui->lineEditGasPrice->setProperty("cssClass", "edit-primary-multi-book");
    ui->lineEditGasPrice->setValidator(new QRegExpValidator(QRegExp("[0-9]+"), ui->lineEditGasPrice));
    ui->lineEditGasPrice->setPlaceholderText(tr("Enter gas price"));

    /*Button*/
    ui->pbnClearAll->setProperty("cssClass","btn-secundary-small");
    ui->pbnCreateContract->setProperty("cssClass","btn-secundary-small");

    /*Checkbox*/
    //ui->checkBoxBroadcast;
    //ui->checkBoxChange

    connect(ui->pbnClearAll, SIGNAL(clicked(bool)), this, SLOT(onClearAll()));
    connect(ui->pbnCreateContract, SIGNAL(clicked(bool)), this, SLOT(onCreateContract()));
    connect(ui->checkBoxChange, SIGNAL(stateChanged(int)), this, SLOT(onChange(int)));
    connect(ui->checkBoxBroadcast, SIGNAL(stateChanged(int)), this, SLOT(onBroadcast(int)));
    connect(ui->lineEditGasPrice, SIGNAL(editingFinished()), this, SLOT(onGasPrice()));
    connect(ui->lineEditGasLimit, SIGNAL(editingFinished()), this, SLOT(onGasLimit()));
    connect(ui->textEditBytecode, SIGNAL(textChanged()), this, SLOT(onBytecode()));

    gasLimit = DEFAULT_GAS_LIMIT_OP_CREATE;
    gasPrice = 40;//(minGasPrice>DEFAULT_GAS_PRICE)?minGasPrice:DEFAULT_GAS_PRICE;
}

CreateContract::~CreateContract()
{
    delete ui;
}

void CreateContract::onClearAll()
{
    ui->textEditBytecode->clear();
    bytecode.clear();
    ui->lineEditGasPrice->clear();
    gasPrice = 0;
    ui->lineEditGasLimit->clear();
    gasLimit = 0;
    ui->lineEditSenderAddress->clear();
    address.clear();
    ui->checkBoxChange->setChecked(false);
    ui->checkBoxBroadcast->setChecked(false);
}

void CreateContract::onBytecode() {

}

void CreateContract::onGasLimit()
{
    if(!ui->lineEditGasLimit->text().isEmpty()) {
        uint64_t limit = ui->lineEditGasLimit->text().toInt();
        uint64_t blockGasLimit = 40000000;//= qtumDGP.getBlockGasLimit(::ChainActive().Height());
        if (limit > blockGasLimit) {
            informError(tr("Invalid value for gasLimit. Maximum is: %1").arg(
                    QString::fromStdString(i64tostr(blockGasLimit))));
            return;
        }
        if (limit < MINIMUM_GAS_LIMIT) {
            informError(tr("Invalid value for gasLimit. Minimum is: %1").arg(
                    QString::fromStdString(i64tostr(MINIMUM_GAS_LIMIT))));
            return;
        }
        if (limit <= 0) {
            informError(tr("Invalid value for gasLimit."));
            return;
        }

        gasLimit = ui->lineEditGasLimit->text().toInt();
    }
}

void CreateContract::onGasPrice()
{
    if(!ui->lineEditGasPrice->text().isEmpty()) {
        CAmount nGasPrice = ui->lineEditGasPrice->text().toInt();
        CAmount maxRpcGasPrice = 10000000;//gArgs.GetArg("-rpcmaxgasprice", MAX_RPC_GAS_PRICE);
        uint64_t minGasPrice = 40;//CAmount(qtumDGP.getMinGasPrice(::ChainActive().Height()));
        if (nGasPrice <= 0) {
            informError(tr("Invalid value for gasPrice."));
            return;
        }
        if (nGasPrice > (int64_t) maxRpcGasPrice) {
            informError(tr("Invalid value for gasPrice. Maximum allowed in RPC calls is: ").arg(
                    QString::fromStdString(FormatMoney(maxRpcGasPrice))));
            return;
        }
        if (nGasPrice < (int64_t) minGasPrice) {
            informError(tr("Invalid value for gasPrice. Minimum is: ").arg(
                    QString::fromStdString(FormatMoney(minGasPrice))));
            return;
        }

        gasPrice = ui->lineEditGasPrice->text().toInt();
    }
}

void CreateContract::onLineEditClicked() {
    int contactsSize = walletModel->getAddressTableModel()->sizeRecv();
    if(contactsSize == 0) {
        inform(tr( "No receive addresses available, you can go to the receive screen and create some there!"));
        return;
    }

    int height = 45;
    height = (contactsSize < 4) ? height * contactsSize + 25 : height * 4 + 25;
    int width = ui->lineEditSenderAddress->width();

    if(!menuContacts){
        menuContacts = new ContactsDropdown(
                width,
                height,
                this
        );
        menuContacts->setGraphicsEffect(0);
        connect(menuContacts, &ContactsDropdown::contactSelected, [this](QString address, QString label){
            ui->lineEditSenderAddress->setText(address);
            this->address = address;

            ui->lineEditSenderAddress->removeAction(btnUpAddressContact);
            ui->lineEditSenderAddress->addAction(btnAddressContact, QLineEdit::TrailingPosition);
        });
    }

    if(menuContacts->isVisible())
    {
        menuContacts->hide();
        ui->lineEditSenderAddress->removeAction(btnUpAddressContact);
        ui->lineEditSenderAddress->addAction(btnAddressContact, QLineEdit::TrailingPosition);
        return;
    }
    else
    {
        ui->lineEditSenderAddress->removeAction(btnAddressContact);
        ui->lineEditSenderAddress->addAction(btnUpAddressContact, QLineEdit::TrailingPosition);
    }

    menuContacts->setWalletModel(walletModel, AddressTableModel::Receive);
    menuContacts->resizeList(width, height);
    menuContacts->setStyleSheet(styleSheet());
    menuContacts->adjustSize();

    QPoint pos;
    pos = ui->lineEditSenderAddress->pos();
    pos.setY((pos.y() + ui->lineEditSenderAddress->height() * 3));

    pos.setX(pos.x() + 9);
    pos.setY(pos.y() + height + 8);

    menuContacts->move(pos);
    menuContacts->show();
}

void CreateContract::onBroadcast(int state)
{
    if(ui->checkBoxBroadcast->checkState() == Qt::Checked)
        isBroadcast = true;
    else
        isBroadcast = false;
}

void CreateContract::onChange(int state)
{
    if(ui->checkBoxChange->checkState() == Qt::Checked)
        isChange = true;
    else
        isChange = false;
}

void CreateContract::onCreateContract()
{
    std::cout << "craete contract" << std::endl;

    bool fHasSender = false;
    CTxDestination senderAddress;
    if(!ui->lineEditSenderAddress->text().isEmpty()) {
        senderAddress = DecodeDestination(address.toStdString());
        if (senderAddress.index() == 0) {
            informError(tr("Invalid Qtum address to send from."));
            return;
        }

        if (std::get_if<PKHash>(&senderAddress) == 0) {
            informError(tr("Invalid contract sender address. Only P2PK and P2PKH allowed."));
            return;
        } else
            fHasSender = true;
    }

    bytecode = ui->textEditBytecode->toPlainText();
    if (bytecode.toStdString().size() % 2 != 0 || !CheckHex(bytecode.toStdString()))
    {
        informError(tr("Invalid data, bytecode not hex."));
        return;
    }
    /*if(gasLimit.isEmpty())
    {
        informError(tr("Invalid data, line Gas Limit is not filled."));
        return;
    }
    if(gasPrice.isEmpty())
    {
        informError(tr("Invalid data, line Gas Price is not filled."));
        return;
    }*/
    /*if(this->senderAddress.isEmpty())
    {
        informError(tr("Invalid data, line Sender Address is not filled."));
        return;
    }*/

    std::unique_ptr<CCoinControl> coinControl;
    CTxDestination signSenderAddress = CNoDestination();

    if(fHasSender)
    {
        // Find a UTXO with sender address
        std::vector<COutput> vecOutputs;

        coinControl.reset(new CCoinControl());
        coinControl->fAllowOtherInputs=true;

        assert(pwalletMain != NULL);
        pwalletMain->AvailableCoins(&vecOutputs, false, NULL, true);

        for (const COutput& out : vecOutputs) {
            CTxDestination destAdress;
            const CScript scriptPubKey = out.tx->vout[out.i].scriptPubKey;
            bool fValidAddress = ExtractDestination(scriptPubKey, destAdress);

            if (!fValidAddress || !(senderAddress == destAdress))
                continue;

            coinControl->Select(COutPoint(out.tx->GetHash(),out.i));
            break;
        }

        if (coinControl->HasSelected()) {
            // Change to the sender
            if(isChange){
                coinControl->destChange=senderAddress;
            }
            signSenderAddress = senderAddress;
        } else
            coinControl.reset();

    } else {
        coinControl.reset();
        SetDefaultSignSenderAddress(pwalletMain, signSenderAddress);
    }

    //EnsureWalletIsUnlocked()
    if(pwalletMain->IsLocked() || pwalletMain->fWalletUnlockAnonymizeOnly)
    {
        informError(tr("Please enter the wallet passphrase with walletpassphrase first."));
        return;
    }

    CAmount nGasFee= gasPrice * gasLimit;
    CAmount curBalance = pwalletMain->GetBalance();

    // Check amount
    if (nGasFee <= 0)
    {
        informError(tr("Invalid amount for gas fee."));
        return;
    }

    if (nGasFee > curBalance)
    {
        informError(tr("Insufficient funds."));
        return;
    }

    // Build OP_EXEC script
    CScript scriptPubKey = CScript() << CScriptNum(VersionVM::GetEVMDefault().toRaw()) << CScriptNum(gasLimit) << CScriptNum(gasPrice) << ParseHex(bytecode.toStdString()) <<OP_CREATE;
    if(IsValidDestination(signSenderAddress))
    {
        CKeyID key_id = GetKeyForDestination(*pwalletMain, signSenderAddress);
        CKey key;
        if (!pwalletMain->GetKey(key_id, key)) {
            informError(tr("Private key not available."));
            return;
        }
        std::vector<unsigned char> scriptSig;
        scriptPubKey = (CScript() << CScriptNum(addresstype::PUBKEYHASH) << ToByteVector(key_id) << ToByteVector(scriptSig) << OP_SENDER) + scriptPubKey;
    }
    else
    {
        informError(tr("Sender address fail to set for OP_SENDER."));
        return;
    }

    // Create and send the transaction
    CAmount nFeeRequired = 0;
    std::string strError;
    CAmount nValue = 0;

    //CTransactionRef tx;
    // make our change address
    CReserveKey reservekey(pwalletMain);
    CWalletTx wtx;

    if (nGasFee > 0) nValue = nGasFee;
    std::vector<std::pair<CScript, CAmount> > vecSend;
    vecSend.push_back(std::make_pair(scriptPubKey, nValue));

    if (!pwalletMain->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, strError, coinControl.get(), ALL_COINS, false, nGasFee, false, true, true, signSenderAddress)) {
        if (nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf("This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        informError(tr(strError.c_str()));
        return;
    }

    CTxDestination txSenderDest;
    GetSenderDest(pwalletMain, wtx, txSenderDest);

    if (fHasSender && !(senderAddress == txSenderDest)){
        informError(tr("Sender could not be set, transaction was not committed!"));
        return;
    }

    UniValue result(UniValue::VOBJ);
    if(isBroadcast){
        CValidationState state;
        if (!pwalletMain->CommitTransaction(wtx, reservekey))
        {
            informError(tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of the wallet and coins were spent in the copy but not marked as spent here."));
            return;
        }

        std::string txId=wtx.GetHash().GetHex();
        result.pushKV("txid", txId);

        CTxDestination txSenderAdress(txSenderDest);
        CKeyID keyid = GetKeyForDestination(*pwalletMain, txSenderAdress);

        result.pushKV("sender", EncodeDestination(txSenderAdress));
        result.pushKV("hash160", HexStr(valtype(keyid.begin(),keyid.end())));

        std::vector<unsigned char> SHA256TxVout(32);
        std::vector<unsigned char> contractAddress(20);
        std::vector<unsigned char> txIdAndVout(wtx.GetHash().begin(), wtx.GetHash().end());
        uint32_t voutNumber=0;
        for (const CTxOut& txout : wtx.vout) {
            if(txout.scriptPubKey.HasOpCreate()){
                std::vector<unsigned char> voutNumberChrs;
                if (voutNumberChrs.size() < sizeof(voutNumber))voutNumberChrs.resize(sizeof(voutNumber));
                std::memcpy(voutNumberChrs.data(), &voutNumber, sizeof(voutNumber));
                txIdAndVout.insert(txIdAndVout.end(),voutNumberChrs.begin(),voutNumberChrs.end());
                break;
            }
            voutNumber++;
        }
        CSHA256().Write(txIdAndVout.data(), txIdAndVout.size()).Finalize(SHA256TxVout.data());
        CRIPEMD160().Write(SHA256TxVout.data(), SHA256TxVout.size()).Finalize(contractAddress.data());
        result.pushKV("address", HexStr(contractAddress));
    }
    else
    {
        std::string strHex = EncodeHexTx(wtx);
        result.pushKV("raw transaction", strHex);
    }

    informWarning(QString::fromStdString(result.get_str()));
    std::cout << result.get_str() << std::endl;
}

bool CreateContract::SetDefaultSignSenderAddress(CWallet* const pwallet, CTxDestination& destAddress)
{
    // Set default sender address if none provided
    // Select any valid unspent output that can be used for contract sender address
    std::vector<COutput> vecOutputs;

    assert(pwallet != NULL);
    pwallet->AvailableCoins(&vecOutputs, false, NULL, true);

    for (const COutput& out : vecOutputs) {
        const CScript& scriptPubKey = out.tx->vout[out.i].scriptPubKey;
        bool fValidAddress = ExtractDestination(scriptPubKey, destAddress)
                             && std::get_if<PKHash>(&destAddress) != 0;

        if (!fValidAddress)
            continue;
        break;
    }

    return !std::get_if<CNoDestination>(&destAddress);
}

CKeyID CreateContract::GetKeyForDestination(const CCryptoKeyStore& store, const CTxDestination& dest)
{
    // Only supports destinations which map to single public keys, i.e. P2PKH,
    // P2WPKH, and P2SH-P2WPKH.
    if (auto id = std::get_if<PKHash>(&dest)) {
        return ToKeyID(*id);
    }
    if (auto witness_id = std::get_if<WitnessV0KeyHash>(&dest)) {
        return ToKeyID(*witness_id);
    }
    if (auto script_hash = std::get_if<ScriptHash>(&dest)) {
        CScript script;
        CScriptID script_id(*script_hash);
        CTxDestination inner_dest;
        if (store.GetCScript(script_id, script) && ExtractDestination(script, inner_dest)) {
            if (auto inner_witness_id = std::get_if<WitnessV0KeyHash>(&inner_dest)) {
                return ToKeyID(*inner_witness_id);
            }
        }
    }
    return CKeyID();
}

bool CreateContract::GetSenderDest(CWallet * const pwallet, const CTransaction& tx, CTxDestination& txSenderDest)
{
    // Initialize variables
    CScript senderPubKey;

    // Get sender destination
    if(tx.HasOpSender())
    {
        // Get destination from the outputs
        for(CTxOut out : tx.vout)
        {
            if(out.scriptPubKey.HasOpSender())
            {
                ExtractSenderData(out.scriptPubKey, &senderPubKey, 0);
                break;
            }
        }
    }
    else
    {
        // Get destination from the inputs
        senderPubKey = pwallet->mapWallet.at(tx.vin[0].prevout.hash).tx->vout[tx.vin[0].prevout.n].scriptPubKey;
    }

    // Extract destination from script
    return ExtractDestination(senderPubKey, txSenderDest);
}