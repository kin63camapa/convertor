#include "ticketpreview.h"
#include "ui_ticketpreview.h"
#include "ticket.h"

TicketPreview::TicketPreview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TicketPreview)
{
    ui->setupUi(this);
    index=0;

}

TicketPreview::~TicketPreview()
{
    delete ui;
}

void TicketPreview::showTicket(int i)
{
    index=i;
    qDebug() << i;
    ui->customer_id->setText(list.at(i).customer_id);
    ui->customer_user_id->setText(list.at(i).customer_id);
    ui->dateTimeEdit->setDateTime(list.at(i).creationTime);
    ui->email->setText(list.at(i).email);
    ui->number->setValue(list.at(i).ticket_number);
    ui->realID->setValue(list.at(i).ID);
    ui->theme->setText(list.at(i).theme);
    ui->body->clear();
    foreach (TICKET::message m , list.at(i).messages)
    {
        ui->body->textCursor().insertText(m.time.toString("hh:mm:ss dd.MM.yyyy\n"));
        ui->body->textCursor().insertText(QString::fromUtf8(m.text.toAscii()));
    }

}

void TicketPreview::on_nextBtn_clicked()
{
    if(index<=list.size()-1) showTicket(++index);
}

void TicketPreview::on_prewBtn_clicked()
{
    if(list.size()>index) showTicket(--index);
}
