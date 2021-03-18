// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The BTCU developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MNROW_H
#define MNROW_H

#include <QWidget>

namespace Ui {
class MNRow;
}

class MNRow : public QWidget
{
    Q_OBJECT

public:
    explicit MNRow(QWidget *parent = nullptr);
    ~MNRow();

    void updateView(std::string name, std::string address, double leasing, int blockHeight, QString type, double profit);

Q_SIGNALS:
    void onMenuClicked();

private Q_SLOTS:
   void onPbnMenuClicked();
private:
    Ui::MNRow *ui;
};

#endif // MNROW_H
