#include "sqlconnectdialog.h"
#include "ui_sqlconnectdialog.h"

#include <QMessageBox>



SQLConnectDialog::SQLConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SQLConnectDialog)
{
    ui->setupUi(this);
    if (!QSqlDatabase::drivers().contains("QMYSQL"))
        QMessageBox::information(this, tr("No mysql database drivers found"),
                                 tr("This super programm requires mysql database driver."
                                    "Please check the documentation how to build the "
                                    "Qt MySQL plugin."));
}

SQLConnectDialog::~SQLConnectDialog()
{
    delete ui;
}

QSqlDatabase SQLConnectDialog::connect(QSqlDatabase db)
{
    tmp=db;
    this->exec();
    return tmp;
}

void SQLConnectDialog::on_ConnectButton_clicked()
{
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QMYSQL");
    //db.setDatabaseName("");
    db.setHostName(ui->HostEdit->text());
    db.setPort(ui->Port->value());
    if (!db.open(ui->UserEdit->text(),ui->PassEdit->text()))
        QMessageBox(QMessageBox::Critical,QString::fromUtf8("Апшипка"),db.lastError().text()).exec();
    else
    {
        tmp = db;
        close();
    }
}
