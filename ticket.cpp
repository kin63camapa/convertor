#include "ticket.h"

TICKET::TICKET(QObject *parent) :
    QObject(parent)
{
    clear();
}

bool TICKET::operator ==(int ticket_number)
{
    return this->ticket_number == ticket_number;
}

void TICKET::clear()
{
    ID = 0;
    ticket_number = 0;
    isNew = false;
    QDateTime clearDT;
    time=clearDT;
    theme.clear();
    text.clear();
    email.clear();
    customer_user_id.clear();
    customer_id.clear();
}
