#ifndef CREATECONTRACT_H
#define CREATECONTRACT_H

#include <QWidget>
#include "qt/btcu/pwidget.h"
#include "qt/btcu/contactsdropdown.h"
#include "qt/btcu/sendmultirow.h"
#include "coincontrol.h"

namespace Ui {
class CreateContract;
}

class CreateContract : public PWidget
{
    Q_OBJECT

public:
    explicit CreateContract(BTCUGUI *parent = nullptr);
    ~CreateContract();

private:
    bool SetDefaultSignSenderAddress(CWallet* const pwallet, CTxDestination& destAddress);
    CKeyID GetKeyForDestination(const CCryptoKeyStore& store, const CTxDestination& dest);
    bool GetSenderDest(CWallet * const pwallet, const CTransaction& tx, CTxDestination& txSenderDest);

private Q_SLOTS:
    void onLineEditClicked();
    void onClearAll();
    void onCreateContract();
    void onChange(int state);
    void onBroadcast(int state);
    void onGasPrice();
    void onGasLimit();
    void onBytecode();

private:
    Ui::CreateContract *ui;

    ContactsDropdown *menuContacts = nullptr;
    SendMultiRow* sendMultiRow;
    QString currentMN;
    ContactsDropdown *menuMasternodes = nullptr;
    QAction *btnAddressContact = nullptr;
    QAction *btnUpAddressContact = nullptr;

    bool isChange;
    bool isBroadcast;
    uint64_t gasPrice;
    uint64_t gasLimit;
    QString address;
    QString bytecode;
};

#endif // CREATECONTRACT_H
