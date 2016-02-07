#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::on_fBtn_clicked()
{
    if (ui->radioButton->isChecked()) emit fnd(byNum,QString::number(ui->numberBox->value()));
    if (ui->radioButton_2->isChecked()) emit fnd(byId,QString::number(ui->idBox->value()));
    if (ui->radioButton_3->isChecked()) emit fnd(byPos,QString::number(ui->posBox->value()));
    if (ui->radioButton_4->isChecked()) emit fnd(byStr,ui->lineEdit->text());
}
