// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCU_CORE_NEW_GUI_BTCUGUI_H
#define BTCU_CORE_NEW_GUI_BTCUGUI_H

#if defined(HAVE_CONFIG_H)
#include "config/btcu-config.h"
#endif

#include <QMainWindow>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QLabel>

#include "qt/btcu/navmenuwidget.h"
#include "qt/btcu/topbar.h"
#include "qt/btcu/dashboardwidget.h"
#include "qt/btcu/send.h"
#include "qt/btcu/receivewidget.h"
#include "qt/btcu/addresseswidget.h"
#include "qt/btcu/privacywidget.h"
#include "qt/btcu/coldstakingwidget.h"
#include "qt/btcu/leasingwidget.h"
#include "qt/btcu/masternodeswidget.h"
#include "qt/btcu/validatorwidget.h"
#include "qt/btcu/createcontract.h"
#include "qt/btcu/snackbar.h"
#include "qt/btcu/settings/settingswidget.h"
#include "qt/rpcconsole.h"
//#include "qt/btcu/createmasternodewidget.h"
#include "qt/btcu/leasingstatisticswidget.h"
#include <QProgressDialog>

#ifdef Q_OS_MAC
#include "qt/macos_appnap.h"
#endif

class ClientModel;
class NetworkStyle;
class Notificator;
class WalletModel;


/**
  BTCU GUI main class. This class represents the main window of the BTCU UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class BTCUGUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;

    explicit BTCUGUI(const NetworkStyle* networkStyle, QWidget* parent = 0);
    ~BTCUGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel* clientModel);


    void resizeEvent(QResizeEvent *event) override;
    void showHide(bool show);
    int getNavWidth();

Q_SIGNALS:
    void themeChanged(bool isLightTheme, QString& theme);
    void windowResizeEvent(QResizeEvent* event);
public Q_SLOTS:
    void changeTheme(bool isLightTheme);
    void goToDashboard();
    void goToSend();
    void goToReceive();
    void goToAddresses();
    void goToPrivacy();
    void goToMasterNodes();
    void goToColdStaking();
    void goToLeasing();
    void goToValidator();
    void goToContract();
    void goToSettings();

   void goToCreateMasternode();
   void goToCreateValidator();
   void goToLeasingStatistics();

    void connectActions();

    void showProgress(const QString& title, int nProgress);

    /** Get restart command-line parameters and request restart */
    void handleRestart(QStringList args);

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] style     modality and style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
       @param[in] ret       pointer to a bool that will be modified to whether Ok was clicked (modal only)
    */
    void message(const QString& title, const QString& message, unsigned int style, bool* ret = nullptr);
    void messageInfo(const QString& message);
    void messageInfo(const QString& message, int Type);
    bool execDialog(QDialog *dialog, int xDiv = 3, int yDiv = 5);
    /** Open FAQ dialog **/
    void openFAQ(int section = 0);

    /** Show incoming transaction notification for new transactions. */
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
#ifdef ENABLE_WALLET
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    bool addWallet(const QString& name, WalletModel* walletModel);
    bool setCurrentWallet(const QString& name);
    void removeAllWallets();
#endif // ENABLE_WALLET

protected:

    void changeEvent(QEvent* e) override;
    void closeEvent(QCloseEvent* event) override;

    /*
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    bool eventFilter(QObject* object, QEvent* event);
     */

private:
    bool enableWallet;
    ClientModel* clientModel = nullptr;
    QProgressDialog* pProgressDialog = nullptr;

    // Actions
    QAction* quitAction = nullptr;
    QAction* toggleHideAction = nullptr;


    // Frame
    NavMenuWidget *navMenu = nullptr;
    TopBar *topBar = nullptr;
    QStackedWidget *stackedContainer = nullptr;

    DashboardWidget *dashboard = nullptr;
    SendWidget *sendWidget = nullptr;
    ReceiveWidget *receiveWidget = nullptr;
    AddressesWidget *addressesWidget = nullptr;
    PrivacyWidget *privacyWidget = nullptr;
    MasterNodesWidget *masterNodesWidget = nullptr;
    ColdStakingWidget *coldStakingWidget = nullptr;
    LeasingWidget *leasingWidget = nullptr;
    ValidatorWidget *validatorWidget = nullptr;
    CreateContract *contractWidget = nullptr;
    SettingsWidget* settingsWidget = nullptr;
    //CreateMasterNodeWidget* createMasterNode = nullptr;
    //CreateValidatorWidget * createValidator = nullptr;
    LeasingStatisticsWidget* LeasingStatistics = nullptr;

    SnackBar *snackBar = nullptr;

    RPCConsole* rpcConsole = nullptr;

    //
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayIconMenu = nullptr;
    Notificator* notificator = nullptr;

    QLabel *op = nullptr;
    bool opEnabled = false;

#ifdef Q_OS_MAC
    CAppNapInhibitor *m_app_nap_inhibitor = nullptr;
#endif

    /** Create the main UI actions. */
    void createActions(const NetworkStyle* networkStyle);
    /** Create system tray icon and notification */
    void createTrayIcon(const NetworkStyle* networkStyle);
    /** Create system tray menu (or setup the dock menu) */
    void createTrayIconMenu();

    void showTop(QWidget *view);
    bool openStandardDialog(QString title = "", QString body = "", QString okBtn = "OK", QString cancelBtn = "CANCEL");

    /** Connect core signals to GUI client */
    void subscribeToCoreSignals();
    /** Disconnect core signals from GUI client */
    void unsubscribeFromCoreSignals();

private Q_SLOTS:
    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);

    /** Simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    /** called by a timer to check if fRequestShutdown has been set **/
    void detectShutdown();

#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif

Q_SIGNALS:
    /** Signal raised when a URI was entered or dragged to the GUI */
    void receivedURI(const QString& uri);
    /** Restart handling */
    void requestedRestart(QStringList args);

};


#endif //BTCU_CORE_NEW_GUI_BTCUGUI_H
