#include "ticket.h"

TICKET::TICKET(QObject *parent) :
    QObject(parent)
{
    clear();
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
