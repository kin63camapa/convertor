#ifndef SQLCONNECTDIALOG_H
#define SQLCONNECTDIALOG_H

#include <QDialog>
#include <QtSql>


namespace Ui {
class SQLConnectDialog;
}

class SQLConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SQLConnectDialog(QWidget *parent = 0);
    ~SQLConnectDialog();
    QSqlDatabase connect(QSqlDatabase db);
private slots:
    void on_ConnectButton_clicked();

private:
    Ui::SQLConnectDialog *ui;
    QSqlDatabase tmp;
};

#endif // SQLCONNECTDIALOG_H
