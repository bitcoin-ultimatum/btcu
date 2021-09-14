#ifndef VALIDATORROW_H
#define VALIDATORROW_H

#include <QWidget>

namespace Ui {
class ValidatorRow;
}

class ValidatorRow : public QWidget
{
    Q_OBJECT

public:
    explicit ValidatorRow(QWidget *parent = nullptr);
    ~ValidatorRow();

    void updateView(QString name, QString pubKey, QString vin);
    void setVoteVisible(bool isVisible);

private Q_SLOTS:
    void onCheckBoxStateChanged();

Q_SIGNALS:
    void voted(QString pubKey, bool vote);

private:
    Ui::ValidatorRow *ui;
};

#endif // VALIDATORROW_H
