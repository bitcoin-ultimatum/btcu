// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/btcu/leasingwidget.h"
#include "qt/btcu/forms/ui_leasingwidget.h"
#include "qt/btcu/qtutils.h"
#include "amount.h"
#include "guiutil.h"
#include "qt/btcu/requestdialog.h"
#include "qt/btcu/tooltipmenu.h"
#include "qt/btcu/furlistrow.h"
#include "qt/btcu/sendconfirmdialog.h"
#include "qt/btcu/addnewcontactdialog.h"
#include "qt/btcu/guitransactionsutils.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "coincontroldialog.h"
#include "coincontrol.h"
#include "qt/btcu/lrow.h"
#include "qt/btcu/defaultdialog.h"
#include "qt/btcu/multileasingdialog.h"
#include <QScreen>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <iostream>
#include "key_io.h"

#define DECORATION_SIZE 70
#define NUM_ITEMS 3
#define LOAD_MIN_TIME_INTERVAL 15
#define REQUEST_LOAD_TASK 1

class LeasingHolder : public FurListRow<QWidget*>
{
public:
    explicit LeasingHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    LRow* createHolder(int pos) override{
        if (!cachedRow) cachedRow.reset(new LRow());
        return cachedRow.get();
    }

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override{
        LRow *row = static_cast<LRow*>(holder);
        row->updateState(isLightTheme, isHovered, isSelected);

        QString address = index.data(Qt::DisplayRole).toString();
        QString label = index.sibling(index.row(), LeasingModel::ADDRESS_LABEL).data(Qt::DisplayRole).toString();
        if (label.isEmpty()) {
            label = "Address with no label";
        }
        QString amountStr = index.sibling(index.row(), LeasingModel::TOTAL_AMOUNT).data(Qt::DisplayRole).toString();
        QString rewardStr = index.sibling(index.row(), LeasingModel::TOTAL_REWARD).data(Qt::DisplayRole).toString();
        bool isLeaser = index.sibling(index.row(), LeasingModel::IS_LEASER).data(Qt::DisplayRole).toBool();
        row->updateView(address, label, isLeaser, amountStr, rewardStr);
        row->showMenuButton(true);
    }

    QColor rectColor(bool isHovered, bool isSelected) override{
        return getRowColor(isLightTheme, isHovered, isSelected);
    }

    ~LeasingHolder() = default;

    bool isLightTheme;
private:
    std::unique_ptr<LRow> cachedRow;
};

LeasingWidget::LeasingWidget(BTCUGUI* parent) :
    PWidget(parent),
    ui(new Ui::LeasingWidget),
    isLoading(false)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());


    /* Containers */
    /*setCssProperty(ui->left, "container");
    ui->left->setContentsMargins(0,20,0,0);
    setCssProperty(ui->right, "container-right");
    ui->right->setContentsMargins(0,10,0,20);*/
   setCssProperty(ui->containerLeft, "container-border");
   setCssProperty(ui->containerRight, "container-border");
   setCssProperty(ui->containerbottom, "container-border");
   //initCssEditLine(ui->containerBTCU);
   setCssProperty(ui->scrollArea, "container");
   //ui->containerSend->setGraphicsEffect(0);

   setCssProperty(ui->pushImgEmpty, "img-empty");
    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
   setCssTitleScreen(ui->labelLeasing);
   setCssTitleScreen(ui->labelTopLeasing);
   setCssTitleScreen(ui->labelHistory);
   setCssSubtitleScreen(ui->labelEditTitle);
   setCssProperty(ui->labelStatus, "text-body2-grey");
   setCssProperty(ui->labelAmount, "text-body2-grey");
   setCssProperty(ui->labelMasternode, "text-body2-grey");
   setCssProperty(ui->labelAddress, "text-body2-grey");
   setCssProperty(ui->labelProfit, "text-body2-grey");

   /*Line Edit*/
   ui->lineEditOwnerAddress->setPlaceholderText(tr("Optional"));
    setCssProperty(ui->lineEditOwnerAddress, "edit-primary-multi-book");
    ui->lineEditOwnerAddress->setAttribute(Qt::WA_MacShowFocusRect, 0);

   btnOwnerContact = ui->lineEditOwnerAddress->addAction(getIconComboBox(isLightTheme(),false), QLineEdit::TrailingPosition);
   connect(btnOwnerContact, &QAction::triggered, [this](){ onOwnerClicked(); });

   btnUpOwnerContact = ui->lineEditOwnerAddress->addAction(getIconComboBox(isLightTheme(),true), QLineEdit::TrailingPosition);
   connect(btnUpOwnerContact, &QAction::triggered, [this](){ onOwnerClicked(); });

   ui->lineEditOwnerAddress->removeAction(btnUpOwnerContact);

   /* Button*/
   setCssBtnSecondary(ui->pbnCONFIRM);
   setCssBtnPrimary(ui->pbnReset);

   connect(ui->pbnCONFIRM, SIGNAL(clicked()), this, SLOT(onpbnCONFIRM()));
   connect(ui->pbnReset, SIGNAL(clicked()), this, SLOT(onResetForm()));

    // Transactions
    txHolder = new TxViewHolder(isLightTheme());
    txViewDelegate = new FurAbstractListItemDelegate(
            DECORATION_SIZE,
            txHolder,
            this
    );

    setCssProperty(ui->listViewTransaction, "container");
    ui->listViewTransaction->setItemDelegate(txViewDelegate);
    ui->listViewTransaction->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listViewTransaction->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listViewTransaction->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listViewTransaction->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listViewTransaction->setLayoutMode(QListView::LayoutMode::Batched);
    ui->listViewTransaction->setBatchSize(50);
    ui->listViewTransaction->setUniformItemSizes(true);

    connect(ui->listViewTransaction, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    ui->containerHistory->setVisible(true);
    ui->emptyContainer->setVisible(false);

    ui->infoFrame->setVisible(false);

   showHistory();

    // Sort type
    setCssProperty(ui->lineEditSort, "edit-primary-multi-book");
    btnBoxSortType = ui->lineEditSort->addAction(getIconComboBox(isLightTheme(),false), QLineEdit::TrailingPosition);
    connect(btnBoxSortType, &QAction::triggered, [this](){ onBoxSortTypeClicked(); });
    SortEdit* lineEditType = new SortEdit(ui->comboBoxSortType);
    initComboBox(ui->comboBoxSortType, lineEditType);
    connect(lineEditType, &SortEdit::Mouse_Pressed, [this](){ui->comboBoxSortType->showPopup();});

    QSettings settings;
    ui->comboBoxSortType->addItem(tr("All"), TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingSent) | TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingSentToSelf) |
                                                                TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingRecv) |
                                                                TransactionFilterProxy::TYPE(TransactionRecord::LeasingReward) |
                                                                TransactionFilterProxy::TYPE(TransactionRecord::P2LUnlockLeasing) | TransactionFilterProxy::TYPE(TransactionRecord::P2LUnlockOwnLeasing) | TransactionFilterProxy::TYPE(TransactionRecord::P2LReturnLeasing));
    ui->comboBoxSortType->addItem(tr("Leased"), TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingSent) | TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingSentToSelf));
    ui->comboBoxSortType->addItem(tr("Leasings"), TransactionFilterProxy::TYPE(TransactionRecord::P2LLeasingRecv));
    ui->comboBoxSortType->addItem(tr("Leasing rewards"), TransactionFilterProxy::TYPE(TransactionRecord::LeasingReward));
    ui->comboBoxSortType->addItem(tr("Unlock leasings"), TransactionFilterProxy::TYPE(TransactionRecord::P2LUnlockLeasing) | TransactionFilterProxy::TYPE(TransactionRecord::P2LUnlockOwnLeasing) | TransactionFilterProxy::TYPE(TransactionRecord::P2LReturnLeasing));
    ui->comboBoxSortType->setCurrentIndex(0);
    connect(ui->comboBoxSortType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onSortTypeChanged(const QString&)));
    ui->comboBoxSortType->setVisible(false);
    ui->lineEditSort->setText(ui->comboBoxSortType->currentText());
    widgetBoxSortType=new QWidget(this);
    QVBoxLayout* LayoutDigits = new QVBoxLayout(widgetBoxSortType);
    listViewBoxSortType = new QListView();
    LayoutDigits->addWidget(listViewBoxSortType);
    widgetBoxSortType->setGraphicsEffect(0);
    //listViewBoxSort->setGraphicsEffect(shadowEffect);
    listViewBoxSortType->setProperty("cssClass", "container-border-light");
    listViewBoxSortType->setModel(ui->comboBoxSortType->model());
    connect(listViewBoxSortType, SIGNAL(clicked(QModelIndex)), this, SLOT(BoxSortTypeClick(QModelIndex)));
    widgetBoxSortType->hide();

    sendMultiRow = new SendMultiRow(this);
    sendMultiRow->setOnlyLeasingAddressAccepted(true);
    ((QVBoxLayout*)ui->containerSend->layout())->insertWidget(1, sendMultiRow);
    connect(sendMultiRow, &SendMultiRow::onContactsClicked, [this](){ onContactsClicked(); });
}

void LeasingWidget::onBoxSortTypeClicked()
{
    if(widgetBoxSortType->isVisible()){
        widgetBoxSortType->hide();
        btnBoxSortType->setIcon(getIconComboBox(isLightTheme(),false));
        return;
    }
    btnBoxSortType->setIcon(getIconComboBox(isLightTheme(),true));
    QPoint pos = ui->lineEditSort->pos();
    QPoint point = ui->containerLeft->rect().bottomRight();
    widgetBoxSortType->setFixedSize(ui->lineEditSort->width() + 20,200);

    int delta = 422 - ui->containerbottom->mapToParent(ui->lineEditSort->mapToParent(ui->lineEditSort->pos())).y() ? 3 : -3;
    pos.setY(point.y() + ui->lineEditSort->height()*1.5 + delta);
    pos.setX(point.x() + (ui->containerRight->width() - ui->lineEditSort->width()));
    widgetBoxSortType->move(pos);
    widgetBoxSortType->show();
}

void LeasingWidget::onSortTypeChanged(const QString& value){
    if (!filter) return;
    int filterByType = ui->comboBoxSortType->itemData(ui->comboBoxSortType->currentIndex()).toInt();
    filter->setTypeFilter(filterByType);
    ui->listViewTransaction->update();

    if (filter->rowCount() == 0){
        ui->emptyContainer->setVisible(true);
        ui->listViewTransaction->setVisible(false);
    } else {
        ui->emptyContainer->setVisible(false);
        ui->listViewTransaction->setVisible(true);
    }

    // Store settings
    QSettings settings;
    settings.setValue("transactionType", filterByType);
}

void LeasingWidget::BoxSortTypeClick(const QModelIndex &index)
{
    QString value = index.data(0).toString();
    if(value.length() > 22)
    {
        value = value.left(19) + "...";
    }
    ui->lineEditSort->setText(value);
    ui->comboBoxSortType->setCurrentIndex(index.row());
    widgetBoxSortType->hide();
    btnBoxSortType->setIcon(getIconComboBox(isLightTheme(),false));
}

void LeasingWidget::showHistory()
{
   if(bShowHistory)
   {
      ui->containerHistory->setVisible(true);
      ui->emptyContainer->setVisible(false);
   }
   else
   {
      ui->containerHistory->setVisible(false);
      ui->emptyContainer->setVisible(true);
   }
}

void LeasingWidget::onTempADD(QString Address = QString(), QString Name = QString(), QString Amount = QString())
{
   clearLeasingHistoryrow();
   int count = addressesFilter->rowCount();
   if(count <= 0 ) return;
   if(SpacerHistory)
   {
      ui->listViewTransaction->layout()->removeItem(SpacerHistory);
      delete SpacerHistory;
   }
   if(SpacerTop)
   {
      ui->scrollAreaWidgetContentsTop->layout()->removeItem(SpacerTop);
      delete SpacerTop;
   }
   bShowHistory = true;
   for(int i = 0; i < count; i++)
   {
      QModelIndex rIndex = addressesFilter->index(i, TransactionTableModel::ToAddress);
      QString address = rIndex.data(Qt::DisplayRole).toString();
      QString label = rIndex.data(TransactionTableModel::LabelRole).toString();
      if (label.isEmpty()) {
         label = "Address with no label";
      }
      qint64 amount = rIndex.data(TransactionTableModel::AmountRole).toLongLong();
      int status = rIndex.data(TransactionTableModel::StatusRole).toInt();
      n++;
      SpacerTop = new QSpacerItem(20,20,QSizePolicy::Minimum,QSizePolicy::Expanding);
      SpacerHistory = new QSpacerItem(20,20,QSizePolicy::Minimum,QSizePolicy::Expanding);
      ui->listViewTransaction->layout()->addWidget(createLeasinghistoryrow(address, label, QString::number(amount, 10)));
      ui->listViewTransaction->layout()->addItem(SpacerHistory);
      ui->scrollAreaWidgetContentsTop->layout()->addWidget(createLeasingTop(n, address));
      ui->scrollAreaWidgetContentsTop->layout()->addItem(SpacerTop);
   }
   showHistory();
}

QWidget* LeasingWidget::createLeasinghistoryrow(QString Address, QString Name, QString Amount)
{
   QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
   shadowEffect->setColor(QColor(0, 0, 0, 22));
   shadowEffect->setXOffset(0);
   shadowEffect->setYOffset(2);
   shadowEffect->setBlurRadius(6);

   QWidget * Historyrow = new QWidget();
   Historyrow->setObjectName("Historyrow");
   Historyrow->setGraphicsEffect(shadowEffect);
   QHBoxLayout* Layout = new QHBoxLayout(Historyrow);
   Historyrow->setMaximumHeight(50);
   Historyrow->setMinimumHeight(50);
   QLabel* labelStatus = new QLabel(Historyrow);
   labelStatus->setMaximumHeight(35);
   labelStatus->setMinimumHeight(35);
   /*labelStatus->setMaximumWidth(50);
   labelStatus->setMinimumWidth(50);*/
   labelStatus->setText(tr("Going"));
   labelStatus->setProperty("cssClass","text-green");

   QLabel* labelAmount = new QLabel(Historyrow);
   labelAmount->setMaximumHeight(35);
   labelAmount->setMinimumHeight(35);
   labelAmount->setText(Amount + " BTCU");
   setCssSubtitleScreen(labelAmount);

   QLabel* labelAddress = new QLabel(Historyrow);
   labelAddress->setMaximumHeight(35);
   labelAddress->setMinimumHeight(35);
   labelAddress->setText(Address);
   labelAddress->setProperty("cssClass","text-list-amount-send");

   QLabel* labelMasternode = new QLabel(Historyrow);
   labelMasternode->setMaximumHeight(35);
   labelMasternode->setMinimumHeight(35);
   labelMasternode->setText(Name);
   setCssSubtitleScreen(labelMasternode);

   QLabel* labelProfit = new QLabel(Historyrow);
   labelProfit->setMaximumHeight(35);
   labelProfit->setMinimumHeight(35);
   /*labelProfit->setMaximumWidth(75);
   labelProfit->setMinimumWidth(75);*/
   labelProfit->setText(tr("1.50 BTCU"));
   setCssSubtitleScreen(labelProfit);//->setProperty("cssClass","text-list-title1");

   QPushButton* btnMenu = new QPushButton(Historyrow);
   btnMenu->setMaximumHeight(25);
   btnMenu->setMinimumHeight(25);
   btnMenu->setMaximumWidth(30);
   btnMenu->setMinimumWidth(30);
   btnMenu->setProperty("cssClass","btn-menu");
   btnMenu->setFocusPolicy(Qt::NoFocus);
   connect(btnMenu, SIGNAL(clicked()), this, SLOT(onpbnMenuClicked()));


   Layout->addWidget(labelStatus);
   Layout->addWidget(labelAmount);
   Layout->addWidget(labelAddress);
   Layout->addWidget(labelMasternode);
   Layout->addWidget(labelProfit);
   Layout->addWidget(btnMenu);
   Layout->setStretch(0,1);
   Layout->setStretch(1,1);
   Layout->setStretch(2,3);
   Layout->setStretch(3,1);
   Layout->setStretch(4,1);
   Layout->setStretch(5,0);
   Historyrow->setLayout(Layout);
   setCssProperty(Historyrow, "container-border");

   return Historyrow;
}

QWidget* LeasingWidget::createLeasingTop(int Nun, QString Address)
{
   QWidget * Top = new QWidget();
   QHBoxLayout* Layout = new QHBoxLayout(Top);
   Top->setMaximumHeight(37);
   Top->setMinimumHeight(37);
   setCssProperty(Top, "container");

   QLabel* labelnum = new QLabel(Top);
   labelnum->setMaximumHeight(35);
   labelnum->setMinimumHeight(35);
   labelnum->setMaximumWidth(20);
   labelnum->setMinimumWidth(20);
   labelnum->setText(QString::number(Nun, 10));
   setCssSubtitleScreen(labelnum);//->setProperty("cssClass","text-list-title1");
   labelnum->setAlignment(Qt::AlignLeft);

   QLabel* labelAddress = new QLabel(Top);
   labelAddress->setMaximumHeight(35);
   labelAddress->setMinimumHeight(35);
   /*labelAddress->setMaximumWidth(280);
   labelAddress->setMinimumWidth(280);*/
   labelAddress->setText(Address);
   labelAddress->setProperty("cssClass","text-list-amount-send");
   labelAddress->setAlignment(Qt::AlignLeft);

   QLabel* labelProfit = new QLabel(Top);
   labelProfit->setMaximumHeight(35);
   labelProfit->setMinimumHeight(35);
   /*labelProfit->setMaximumWidth(70);
   labelProfit->setMinimumWidth(70);*/
   labelProfit->setText(tr("1.50 BTCU"));
   setCssSubtitleScreen(labelProfit);
   labelProfit->setAlignment(Qt::AlignRight);

   QSpacerItem* Spacer1 = new QSpacerItem(10,0,QSizePolicy::Fixed ,QSizePolicy::Minimum);
   QSpacerItem* Spacer2 = new QSpacerItem(20,0,QSizePolicy::Expanding ,QSizePolicy::Minimum);
   Layout->addWidget(labelnum);
   Layout->addItem(Spacer1);
   Layout->addWidget(labelAddress);
   Layout->addItem(Spacer2);
   Layout->addWidget(labelProfit);
   Layout->setStretch(0,0);
   Layout->setStretch(1,0);
   Layout->setStretch(2,4);
   Layout->setStretch(3,1);
   Layout->setStretch(4,3);
   Top->setLayout(Layout);
   return Top;
}

void LeasingWidget::onResetForm()
{
    ui->lineEditOwnerAddress->clear();
    sendMultiRow->clear();
}

void LeasingWidget::onpbnCONFIRM()
{
   bool ok;
   /*if(walletModel->getBalance() <= (ui->lineEditBTCU->text().toLongLong(&ok, 10)))
   {
      informError(tr("Not enough balance to create a Leasing"));
      return;
   }*/
   onSendClicked();
}


void LeasingWidget::onpbnMenuClicked()
{
   QPushButton* btnMenu = (QPushButton*) sender();
   QPoint pos;
   pos = btnMenu->rect().bottomRight();
   pos = btnMenu->mapToParent(pos);
   pos = btnMenu->parentWidget()->mapToParent(pos);
   pos = btnMenu->parentWidget()->parentWidget()->mapToParent(pos);
   pos.setY(pos.y() +15);

   if(!this->menu)
   {
      this->menu = new TooltipMenu(window, ui->listViewTransaction);
      this->menu->setEditBtnText(tr("More Information"));
      this->menu->setDeleteBtnText(tr("Cancel Leasing"));
      this->menu->setCopyBtnText(tr("Add New Leasing"));
      //connect(this->menu, &TooltipMenu::message, this, &AddressesWidget::message);
      connect(this->menu, SIGNAL(onEditClicked()), this, SLOT(onMoreInformationClicked()));
      //connect(this->menu, SIGNAL(onDeleteClicked()), this, SLOT(onMoreInformationClicked()));
      connect(this->menu, SIGNAL(onCopyClicked()), this, SLOT(onNewLeasingClicked()));
      this->menu->adjustSize();
      this->menu->setFixedHeight(this->menu->height() - 20);
   }else {
      if(this->menu->isVisible())
      {
         this->menu->hide();
         delete this->menu;
         this->menu = nullptr;
         return;
      }
   }
   if(pos.y()+ this->menu->height() > ui->listViewTransaction->height())
   {
      pos = btnMenu->rect().topRight();
      pos = btnMenu->mapToParent(pos);
      pos = btnMenu->parentWidget()->mapToParent(pos);
      pos = btnMenu->parentWidget()->parentWidget()->mapToParent(pos);
      pos.setY(pos.y() - this->menu->height() - 15);
   }
   pos.setX(pos.x() - (DECORATION_SIZE * 1.8));
   menu->move(pos);
   menu->show();
}
void LeasingWidget::onNewLeasingClicked()
{
   MultiLeasingDialog* dialogML = new MultiLeasingDialog(window);
   dialogML->adjustSize();
   showHideOp(true);
   openDialogWithOpaqueBackground(dialogML, window );
   //openDialogWithOpaqueBackgroundY(dialog, window );
   if(dialogML->result())
   {
      DefaultDialog *dialog = new DefaultDialog(window);
      //dialog->setText(tr("Are you sure to lease"), tr("%1 BTCU?\n").arg(ui->lineEditBTCU->text()), tr("yes"),tr("no"));
      //dialog->setText("", tr("Are you sure to lease\n\n%1 BTCU?\n").arg(ui->lineEditBTCU->text()), tr("yes"),tr("no"));
      dialog->adjustSize();
      showHideOp(true);
      openDialogWithOpaqueBackground(dialog, window/*, 2.5*/);
      if(dialog->result())
      {
         informWarning(tr("Confirm new MultiLeasing"));
      }
      else{
         informError(tr("Not confirm new MultiLeasing"));
      }
   }
}

void LeasingWidget::onMoreInformationClicked()
{
   window->goToLeasingStatistics();
}
void LeasingWidget::loadWalletModel(){
    if(walletModel) {
        sendMultiRow->setWalletModel(walletModel);
        txModel = walletModel->getTransactionTableModel();
        leasingModel.reset(new LeasingModel(walletModel->getAddressTableModel(), this));
        //ui->listView->setModel(leasingModel.get());
        //ui->listView->setModelColumn(LeasingModel::ADDRESS);

        addressTableModel = walletModel->getAddressTableModel();
        addressesFilter = new AddressFilterProxyModel(AddressTableModel::Leasing, this);
        addressesFilter->setSourceModel(addressTableModel);
        //ui->listViewLeasingAddress->setModel(addressesFilter);
        //ui->listViewLeasingAddress->setModelColumn(AddressTableModel::Address);

        filter = new TransactionFilterProxy();
        filter->setDynamicSortFilter(true);
        filter->setSortCaseSensitivity(Qt::CaseInsensitive);
        filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        filter->setSortRole(Qt::EditRole);
        filter->setSourceModel(txModel);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);
        int filterByType = ui->comboBoxSortType->itemData(ui->comboBoxSortType->currentIndex()).toInt();
        filter->setTypeFilter(filterByType);

        txHolder->setFilter(filter);

        ui->listViewTransaction->setModel(filter);
        ui->listViewTransaction->setModelColumn(TransactionTableModel::ToAddress);

        if (filter->rowCount() == 0){
            ui->emptyContainer->setVisible(true);
            ui->containerHistory->setVisible(false);
        }
        else{
            ui->containerHistory->setVisible(true);
            ui->emptyContainer->setVisible(false);
        }
        /*else {
            showList(true);
        }*/

        connect(txModel, &TransactionTableModel::txArrived, this, &LeasingWidget::onTxArrived);

        updateDisplayUnit();

        //ui->containerHistoryLabel->setVisible(false);
        //ui->emptyContainer->setVisible(false);
        //ui->listView->setVisible(false);

       onTempADD();
    }

}

void LeasingWidget::onTxArrived(const QString& hash, const bool& isCoinStake, const bool& isCSAnyType, const bool& isLAnyType) {
    /*if (isLAnyType) {
        tryRefreshLeasings();
    }*/
    ui->listViewTransaction->update();
}

void LeasingWidget::walletSynced(bool sync) {
    /*if (this->isChainSync != sync) {
        this->isChainSync = sync;
        tryRefreshLeasings();
    }*/
}

void LeasingWidget::tryRefreshLeasings() {
   int sizeLeasings = addressesFilter->rowCount();
   int tx = txModel->size();
   for(int i = 0; i < sizeLeasings; i++)
   {
      QModelIndex modelIndex = addressesFilter->index(i, LeasingModel::ADDRESS);
      //QString address = index.data(Qt::DisplayRole).toString();
      QString address = modelIndex.sibling(i, LeasingModel::ADDRESS).data(Qt::DisplayRole).toString();
      QString label = modelIndex.sibling(i, LeasingModel::ADDRESS_LABEL).data(Qt::DisplayRole).toString();
      if (label.isEmpty()) {
         label = "Address with no label";
      }
      QString amountStr = modelIndex.sibling(i, LeasingModel::TOTAL_AMOUNT).data(Qt::DisplayRole).toString();
      QString rewardStr = modelIndex.sibling(i, LeasingModel::TOTAL_REWARD).data(Qt::DisplayRole).toString();
      bool isLeaser = modelIndex.sibling(i, LeasingModel::IS_LEASER).data(Qt::DisplayRole).toBool();

      onTempADD(address, label, amountStr);
   }
    // Check for min update time to not reload the UI so often if the node is syncing.
    /*int64_t now = GetTime();
    if (lastRefreshTime + LOAD_MIN_TIME_INTERVAL < now) {
        lastRefreshTime = now;
        refreshLeasings();
    }*/
}

bool LeasingWidget::refreshLeasings(){
    /*if (isLoading) return false;
    isLoading = true;
    return execute(REQUEST_LOAD_TASK);*/
    return true;
}

void LeasingWidget::onLeasingsRefreshed() {
    /*isLoading = false;
    bool hasLeasing = leasingModel->rowCount() > 0;

    updateLeasingTotalLabel();

    // Update list if we are showing that section.
    if (!isInLeasing) {
        showList(hasLeasing);
        //ui->labelLeasingTotal->setVisible(hasLeasing);
        //ui->labelLeasingReward->setVisible(hasLeasing);
    }*/
}

void LeasingWidget::run(int type) {
    /*if (type == REQUEST_LOAD_TASK) {
        leasingModel->updateLeasingList();
        QMetaObject::invokeMethod(this, "onLeasingsRefreshed", Qt::QueuedConnection);
    }*/
}
void LeasingWidget::onError(QString error, int type) {
    /*isLoading = false;
    inform(tr("Error loading leasings: %1").arg(error));
     */
}

void LeasingWidget::onOwnerClicked()
{
    if(menu && menu->isVisible()){
        menu->hide();
    }

    int contactsSize = walletModel->getAddressTableModel()->sizeRecv();
    if(contactsSize == 0) {
        inform(tr( "No receive addresses available, you can go to the receive screen and create some there!"));
        return;
    }

    int height = 45;
    height = (contactsSize < 3) ? height * contactsSize + 25 : height * 3 + 25;
    int width = width = ui->lineEditOwnerAddress->width();

    if (!menuOwner) {
        menuOwner = new ContactsDropdown(
                width,
                height,
                this
        );
        menuOwner->setGraphicsEffect(0);
        connect(menuOwner, &ContactsDropdown::contactSelected, [this](QString address, QString label) {
            ui->lineEditOwnerAddress->setText(address);
            curentAddress = address;
            curentName = label;
        });
    }

    if(menuOwner->isVisible()){
        menuOwner->hide();
        ui->lineEditOwnerAddress->removeAction(btnUpOwnerContact);
        ui->lineEditOwnerAddress->addAction(btnOwnerContact, QLineEdit::TrailingPosition);
        return;
    }
    else
    {
        ui->lineEditOwnerAddress->removeAction(btnOwnerContact);
        ui->lineEditOwnerAddress->addAction(btnUpOwnerContact, QLineEdit::TrailingPosition);
    }

    menuOwner->setWalletModel(walletModel, AddressTableModel::Receive);
    menuOwner->resizeList(width, height);
    menuOwner->setStyleSheet(styleSheet());
    menuOwner->adjustSize();

    QPoint pos;
    pos = ui->lineEditOwnerAddress->pos();
    pos.setY((pos.y() + (ui->lineEditOwnerAddress->height() - 18) * 4));

    pos.setX(pos.x() + 20);
    pos.setY(pos.y() - 14);
    menuOwner->move(pos);
    menuOwner->show();
}

void LeasingWidget::onContactsClicked()
{
    if(menu && menu->isVisible()){
        menu->hide();
    }

    int contactsSize = walletModel->getAddressTableModel()->sizeSend();
    if(contactsSize == 0) {
        inform(tr("No contacts available, you can go to the contacts screen and add some there!"));
        return;
    }

    int height = 45;
    height = (contactsSize < 3) ? height * contactsSize + 25 : height * 3 + 25;
    int width = width = sendMultiRow->getEditWidth();

    if (!menuContacts) {
        menuContacts = new ContactsDropdown(
                width,
                height,
                this
        );
        menuContacts->setGraphicsEffect(0);
        connect(menuContacts, &ContactsDropdown::contactSelected, [this](QString address, QString label) {
            sendMultiRow->setLabel(label);
            sendMultiRow->setAddress(address);
        });
    }

    if(menuContacts->isVisible())
    {
        menuContacts->hide();
        sendMultiRow->updateAction();
        return;
    }

   menuContacts->setWalletModel(walletModel, AddressTableModel::Send);
    menuContacts->resizeList(width, height);
    menuContacts->setStyleSheet(styleSheet());
    menuContacts->adjustSize();

    QPoint pos;
    pos = sendMultiRow->pos();
    pos.setY((pos.y() + (ui->lineEditOwnerAddress->height() - 18) * 4));

    pos.setX(pos.x() + 40);
    pos.setY(pos.y());

    menuContacts->move(pos);
    menuContacts->show();
}

void LeasingWidget::onLeaseSelected(bool leasing){
    /*isInLeasing = leasing;
    if (menu && menu->isVisible()) {
        menu->hide();
    }

    if (menuAddresses && menuAddresses->isVisible()) {
        menuAddresses->hide();
    }*/

    /*if(isInLeasing) {
        ui->btnCoinControl->setVisible(true);
        ui->containerSend->setVisible(true);
        ui->containerBtn->setVisible(true);
        ui->emptyContainer->setVisible(false);
        ui->listView->setVisible(false);
        ui->containerHistoryLabel->setVisible(false);
        ui->btnLeasing->setVisible(false);
        ui->btnMyLeasingAddresses->setVisible(false);
        ui->listViewLeasingAddress->setVisible(false);
        if (ui->rightContainer->count() == 2)
            ui->rightContainer->addItem(spacerDiv.get());
    } else {
        ui->btnCoinControl->setVisible(false);
        ui->containerSend->setVisible(false);
        ui->containerBtn->setVisible(false);
        ui->btnLeasing->setVisible(true);
        showList(leasingModel->rowCount() > 0);
        ui->btnMyLeasingAddresses->setVisible(true);
        // Show address list, if it was previously open
        ui->listViewLeasingAddress->setVisible(isLeasingAddressListVisible);
    }*/
}

void LeasingWidget::updateDisplayUnit() {
   /* if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
    }*/
}

void LeasingWidget::showList(bool show){
    /*ui->emptyContainer->setVisible(!show);
    ui->listView->setVisible(show);
    ui->containerHistoryLabel->setVisible(show);*/
    if (filter->rowCount() == 0){
        ui->emptyContainer->setVisible(true);
        ui->containerHistory->setVisible(false);
    } else {
        ui->emptyContainer->setVisible(false);
        ui->containerHistory->setVisible(true);
    }
}

void LeasingWidget::onSendClicked(){
    if (!walletModel || !walletModel->getOptionsModel() || !verifyWalletUnlocked())
        return;

    if (!walletModel->isLeasingNetworkelyEnabled()) {
       informError(tr("Leasing is networkely disabled"));
        return;
    }

    if (!sendMultiRow->validate()) {
       informError(tr("Invalid entry"));
        return;
    }

    SendCoinsRecipient dest = sendMultiRow->getValue();
    dest.isP2L = true;

    // Amount must be >= minLeasingAmount
    const CAmount& minLeasingAmount = walletModel->getMinLeasingAmount();
    if (dest.amount < minLeasingAmount) {
       informError(tr("Invalid entry, minimum leasing amount is ") +
               BitcoinUnits::formatWithUnit(nDisplayUnit, minLeasingAmount));
        return;
    }

    QString inputOwner = ui->lineEditOwnerAddress->text();
    bool isOwnerEmpty = inputOwner.isEmpty();
    if (isOwnerEmpty)
    {
        std::string strAccount = "";
        std::string purpose = AddressBook::AddressBookPurpose::LEASED;
        CChainParams::Base58Type addrType = CChainParams::PUBKEY_ADDRESS;
        CTxDestination address;
        PairResult r = pwalletMain->getNewAddress(address, strAccount, purpose, addrType);
        inputOwner = QString::fromStdString(EncodeDestination(address));

        CKeyID ownerKey = GetKeyForDestination(*pwalletMain, address);
        if (ownerKey.IsNull())
        {
            informError(tr("Unable to get spend pubkey hash from owneraddress"));
            return;
        }
    }
    if(!walletModel->validateAddress(inputOwner)) {
        informError(tr("Owner address invalid"));
        return;
    }

    bool isLeasingAddressFromThisWallet = walletModel->isMine(dest.address);
    bool isOwnerAddressFromThisWallet = isOwnerEmpty;

    if (!isOwnerAddressFromThisWallet) {
        isOwnerAddressFromThisWallet = walletModel->isMine(inputOwner);

        // Warn the user if the owner address is not from this wallet
        if (!isOwnerAddressFromThisWallet &&
            !ask(tr("ALERT!"),
                    tr("Leasing to an external owner address!\n\n"
                       "The leased coins will NOT be spendable by this wallet.\n"
                       "Spending these coins will need to be done from the wallet or\n"
                       "device containing the owner address.\n\n"
                       "Do you wish to proceed?"))
            ) {
                return;
        }
    }

    // Don't try to lease the balance if both addresses are from this wallet
    if (isLeasingAddressFromThisWallet && isOwnerAddressFromThisWallet) {
       informError(tr("Leasing address corresponds to this wallet, change it to an external node"));
        return;
    }

   DefaultDialog *Defdialog = new DefaultDialog(window);
   Defdialog->setText(tr("Are you sure to lease"), tr("%1 BTCU?\n").arg(dest.amount/100000000.0), tr("yes"),tr("no"));
   Defdialog->setType();
   Defdialog->adjustSize();
   showHideOp(true);
   openDialogWithOpaqueBackground(Defdialog, window );
   if(!Defdialog->result())
   {
      informError(tr("Not confirm new Leasing"));
      return;
   }
    dest.ownerAddress = inputOwner;
    QList<SendCoinsRecipient> recipients;
    recipients.append(dest);

    // Prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus = walletModel->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);

    // process prepareStatus and on error generate message shown to user
    GuiTransactionsUtils::ProcessSendCoinsReturnAndInform(
            this,
            prepareStatus,
            walletModel,
            BitcoinUnits::formatWithUnit(nDisplayUnit, currentTransaction.getTransactionFee()),
            true
    );

    if (prepareStatus.status != WalletModel::OK) {
       informError(tr("Cannot create transaction."));
        return;
    }

    showHideOp(true);
    TxDetailDialog* dialog = new TxDetailDialog(window);
    dialog->setDisplayUnit(nDisplayUnit);
    dialog->setData(walletModel, currentTransaction);
    dialog->adjustSize();
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 5);

    if(dialog->isConfirm()){
        // now send the prepared transaction
        WalletModel::SendCoinsReturn sendStatus = dialog->getStatus();
        // process sendStatus and on error generate message shown to user
        GuiTransactionsUtils::ProcessSendCoinsReturnAndInform(
                this,
                sendStatus,
                walletModel
        );

        if (sendStatus.status == WalletModel::OK) {
            clearAll();
           informWarning(tr("Coins leased"));
           onTempADD();
        }
    }

    dialog->deleteLater();
}

void LeasingWidget::handleTransactionClicked(const QModelIndex &index){

    ui->listViewTransaction->setCurrentIndex(index);
    QModelIndex rIndex = filter->mapToSource(index);

    window->showHide(true);
    TxDetailDialog *dialog = new TxDetailDialog(window, false);
    dialog->setData(walletModel, rIndex);
    connect(dialog, SIGNAL(messageInfo(const QString, int)), window, SLOT(messageInfo(const QString, int)));
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 17);
    disconnect(dialog, SIGNAL(messageInfo(const QString, int)), window, SLOT(messageInfo(const QString, int)));

    // Back to regular status
    ui->listViewTransaction->scrollTo(index);
    ui->listViewTransaction->clearSelection();
    ui->listViewTransaction->setFocus();
    dialog->deleteLater();
}

bool LeasingWidget::validate()
{
   if (!walletModel)
      return false;

   // Check input validity
   bool retval = true;

   // Skip checks for payment request
   if (recipient.paymentRequest.IsInitialized())
      return retval;

   // Check address validity, returns false if it's invalid
   QString address = ui->lineEditOwnerAddress ->text();
   if (address.isEmpty()){
      retval = false;
   } else
      retval = true;//addressChanged(address);

      /*bool isValid = false;
    CAmount value = GUIUtil::parseValue(ui->lineEditBTCU->text(), walletModel->getOptionsModel()->getDisplayUnit(), &isValid);
    value = isValid ? value : -1;
   //CAmount value = getAmountValue(ui->lineEditBTCU->text());

   // Sending a zero amount is invalid
   if (value <= 0) {
      setCssEditLine(ui->containerBTCU, false, true);
      retval = false;
   }

   // Reject dust outputs:
   if (retval && GUIUtil::isDust(address, value)) {
      setCssEditLine(ui->containerBTCU, false, true);
      retval = false;
   }
*/
   return retval;
}
void LeasingWidget::clearLeasingHistoryrow()
{
   QList<QWidget *> listRow = ui->listViewTransaction->findChildren<QWidget *> ("Historyrow");
   int size = listRow.length();
   QWidget * row;
   for(int i = 0; i < size; i++)
   {
      row = listRow.at(i);
      ui->listViewTransaction->layout()->removeWidget(row);
      delete row;
   }
}
void LeasingWidget::clearAll() {
    if (sendMultiRow) sendMultiRow->clear();
    //ui->lineEditOwnerAddress->clear();
    /*if (CoinControlDialog::coinControl) {
        CoinControlDialog::coinControl->SetNull();
        //ui->btnCoinControl->setActive(false);
    }*/
}

void LeasingWidget::onCoinControlClicked(){
   /* if(isInLeasing) {
        if (walletModel->getBalance() > 0) {
            if (!coinControlDialog) {
                coinControlDialog = new CoinControlDialog();
                coinControlDialog->setModel(walletModel);
            } else {
                coinControlDialog->refreshDialog();
            }
            coinControlDialog->exec();
            //ui->btnCoinControl->setActive(CoinControlDialog::coinControl->HasSelected());
        } else {
            inform(tr("You don't have any BTCU to select."));
        }
    }*/
}

void LeasingWidget::onLeasingClicked() {
    //showAddressGenerationDialog();
}

void LeasingWidget::showAddressGenerationDialog() {
   /* if(walletModel && !isShowingDialog) {
        if (!verifyWalletUnlocked()) return;
        isShowingDialog = true;
        showHideOp(true);
        RequestDialog *dialog = new RequestDialog(window);
        dialog->setWalletModel(walletModel);
        dialog->setRequestType(RequestType::Leasing);
        openDialogWithOpaqueBackgroundY(dialog, window, 3.5, 12);
        if (dialog->res == 1){
            inform(tr("URI copied to clipboard"));
        } else if (dialog->res == 2){
            inform(tr("Address copied to clipboard"));
        }
        dialog->deleteLater();
        isShowingDialog = false;
    }*/
}

void LeasingWidget::handleMyLeasingAddressClicked(const QModelIndex &_index) {

    //ui->listViewLeasingAddress->setCurrentIndex(_index);

    /*QRect rect;// = ui->listViewLeasingAddress->visualRect(_index);
    QPoint pos = rect.topRight();
    pos.setX( parentWidget()->rect().right() - (DECORATION_SIZE * 1.5) );
    pos.setY(pos.y() + (DECORATION_SIZE * 2.5));

    QModelIndex rIndex = addressesFilter->mapToSource(_index);

    if(!menuAddresses){
        menuAddresses = new TooltipMenu(window, this);
        menuAddresses->setEditBtnText(tr("Copy"));
        menuAddresses->setDeleteBtnText(tr("Edit"));
        menuAddresses->setCopyBtnVisible(false);
        menuAddresses->adjustSize();
        connect(menuAddresses, &TooltipMenu::message, this, &AddressesWidget::message);
        connect(menuAddresses, SIGNAL(onEditClicked()), this, SLOT(onAddressCopyClicked()));
        connect(menuAddresses, SIGNAL(onDeleteClicked()), this, SLOT(onAddressEditClicked()));
    }else {
        menuAddresses->hide();
    }

    this->addressIndex = rIndex;

    menuAddresses->move(pos);
    menuAddresses->show();*/
}

void LeasingWidget::handleAddressClicked(const QModelIndex &rIndex) {
    //ui->listView->setCurrentIndex(rIndex);
    /*QRect rect;// = ui->listView->visualRect(rIndex);
    QPoint pos = rect.topRight();
    pos.setX(pos.x() - (DECORATION_SIZE * 2));
    pos.setY(pos.y() + (DECORATION_SIZE * 2));

    if(!this->menu){
        this->menu = new TooltipMenu(window, this);
        this->menu->setEditBtnVisible(false);
        this->menu->setDeleteBtnVisible(false);
        this->menu->setCopyBtnText(tr("Edit Label"));
        this->menu->setLastBtnText(tr("Copy address"), 40);
        this->menu->setLastBtnVisible(true);
        this->menu->setMinimumHeight(157);
        this->menu->setFixedHeight(157);
        this->menu->setMinimumWidth(125);
        connect(this->menu, &TooltipMenu::message, this, &AddressesWidget::message);
        connect(this->menu, SIGNAL(onCopyClicked()), this, SLOT(onLabelClicked()));
        connect(this->menu, SIGNAL(onLastClicked()), this, SLOT(onCopyOwnerClicked()));
    }else {
        this->menu->hide();
    }

    this->index = rIndex;
    this->menu->adjustSize();

    menu->move(pos);
    menu->show();*/
}

void LeasingWidget::onAddressCopyClicked() {
    /*GUIUtil::setClipboard(addressIndex.data(Qt::DisplayRole).toString());
    inform(tr("Address copied"));*/
}
void LeasingWidget::onAddressEditClicked() {
    /*onLabelClicked(
            tr("Edit Address Label"),
            addressIndex,
            false
    );*/
}

void LeasingWidget::onCopyOwnerClicked() {
    /*QString owner = index.data(Qt::DisplayRole).toString();
    GUIUtil::setClipboard(owner);
    inform(tr("Address copied"));*/
}

void LeasingWidget::onLabelClicked(){
    /*onLabelClicked(
            tr("Edit Address Label"),
            index,
            false
    );/
}

void LeasingWidget::onLabelClicked(QString dialogTitle, const QModelIndex &index, const bool& isMyLeasingAddresses) {
    /*if(walletModel && !isShowingDialog) {
        isShowingDialog = true;
        showHideOp(true);
        AddNewContactDialog *dialog = new AddNewContactDialog(window);
        dialog->setTexts(dialogTitle);
        QString qAddress = index.data(Qt::DisplayRole).toString();
        dialog->setData(qAddress, walletModel->getAddressTableModel()->labelForAddress(qAddress));
        if (openDialogWithOpaqueBackgroundY(dialog, window, 3.5, 6)) {
            QString label = dialog->getLabel();
            std::string stdString = qAddress.toStdString();
            std::string purpose = walletModel->getAddressTableModel()->purposeForAddress(stdString);
            const CTxDestination address = DecodeDestination(stdString.data());
            if (!label.isEmpty() && walletModel->updateAddressBookLabels(
                    address,
                    label.toUtf8().constData(),
                    purpose
            )) {
                if (isMyLeasingAddresses) {
                    addressTableModel->notifyChange(index);
                } else
                    leasingModel->updateLeasingList();
                inform(tr("Address label saved"));
            } else {
                inform(tr("Error storing address label"));
            }
        }
        isShowingDialog = false;
    }*/
}

void LeasingWidget::onMyLeasingAddressesClicked()
{
    /*isLeasingAddressListVisible = !ui->listViewLeasingAddress->isVisible();
    ui->btnMyLeasingAddresses->setRightIconClass((isLeasingAddressListVisible ?
                                                  "btn-dropdown" : "ic-arrow"), true);
    ui->listViewLeasingAddress->setVisible(isLeasingAddressListVisible);
    if(isLeasingAddressListVisible) {
        ui->rightContainer->removeItem(spacerDiv.get());
        ui->listViewLeasingAddress->update();
    } else {
        ui->rightContainer->addItem(spacerDiv.get());
    }*/
}

void LeasingWidget::changeTheme(bool isLightTheme, QString& theme)
{
    /*static_cast<LeasingHolder*>(leasing->getRowFactory())->isLightTheme = isLightTheme;
    static_cast<AddressHolder*>(addressLeasing->getRowFactory())->isLightTheme = isLightTheme;
    //ui->listView->update();*/
}

void LeasingWidget::updateLeasingTotalLabel()
{
    /*const CAmount total = leasingModel->getTotalAmount();
   // ui->labelLeasingTotal->setText(tr("Total Leasing: %1").arg(
          //  (total == 0) ? "0.00 BTCU" : GUIUtil::formatBalance(total, nDisplayUnit)));

    const CAmount reward = leasingModel->getTotalReward();
    //ui->labelLeasingReward->setText(tr("Total Reward: %2").arg(
        //(reward == 0) ? "0.00 BTCU" : GUIUtil::formatBalance(reward, nDisplayUnit)));
*/
}

LeasingWidget::~LeasingWidget(){
   // ui->rightContainer->removeItem(spacerDiv.get());
    if (sendMultiRow)
        delete sendMultiRow;
}
