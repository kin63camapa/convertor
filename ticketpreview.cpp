#include "ticketpreview.h"
#include "ui_ticketpreview.h"

TicketPreview::TicketPreview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TicketPreview)
{
    ui->setupUi(this);
    connect(this->ui->nextBtn,SIGNAL(clicked()),this,SIGNAL(nextBtnClicked()));
    connect(this->ui->prewBtn,SIGNAL(clicked()),this,SIGNAL(prewBtnClicked()));
    //ui->saveBtm->setDisabled(true);//временная мера не работает сохранение корректно
}

TicketPreview::~TicketPreview()
{
    delete ui;
}

void TicketPreview::on_saveBtm_clicked()
{
    readTicket(false);
}

void TicketPreview::on_injectBtn_clicked()
{
    readTicket(true);
}

void TicketPreview::showTicket(TICKET t,int size)
{
    ui->nextBtn->setDisabled(t.currenIndex==size-1);
    ui->prewBtn->setDisabled(t.currenIndex==0);
    showTicket(t);
}

void TicketPreview::showTicket(TICKET t)
{
    ui->POS->setValue(t.currenIndex);
    ui->customer_id->setText(t.customer_id);
    ui->injectID->setValue(t.injectID);
    ui->customer_user_id->setText(t.customer_user_id);
    ui->dateTimeEdit->setDateTime(t.creationTime);
    ui->email->setText(t.email);
    ui->number->setValue(t.ticket_number);
    ui->realID->setValue(t.ID);
    ui->theme->setText(t.theme);
    ui->body->clear();
    foreach (TICKET::Message m , t.messages)
    {
        ui->body->textCursor().insertText(m.time.toString("hh:mm:ss dd.MM.yyyy\n"));
        ui->body->textCursor().insertText(QString::fromUtf8(m.text.toAscii()));
    }
}

void TicketPreview::readTicket(bool inj)
{
    TICKET t;
    t.currenIndex = ui->POS->value();
    t.customer_id = ui->customer_id->text();
    t.injectID = ui->injectID->value();
    t.customer_user_id = ui->customer_user_id->text();
    t.creationTime=ui->dateTimeEdit->dateTime();
    t.email=ui->email->text();
    t.ticket_number=ui->number->value();
    t.ID=ui->realID->value();
    t.theme=ui->theme->text();
    t.messages.clear();
    TICKET::Message m;
    foreach (QString s, ui->body->toPlainText().split("\n"))
    {
        if (s.contains(QRegExp("\\d\\d\\:\\d\\d\\:\\d\\d\\s\\d\\d\\.\\d\\d\\.\\d\\d\\d\\d")))
        {
            if (!m.time.isNull())
            {
                t.messages.append(m);
            }
            m.time = QDateTime::fromString(s,"hh:mm:ss dd.MM.yyyy");
            m.text.clear();
        }
        else
        {
            m.text+=QString(s.toLocal8Bit()+"\n");
        }
    }
    t.messages.append(m);
    if (inj) emit inject(t);
    else emit save(t);
    showTicket(t);
}
