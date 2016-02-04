#include "ticket.h"

QList<TICKET> list;

//TICKET::TICKET(QObject *parent) :
//    QObject(parent)
//{
//    clear();
//}

TICKET::TICKET()
{
    clear();
}

TICKET::TICKET(const TICKET &other)
{
    ID = other.ID;
    ticket_number = other.ticket_number;
    isNew = other.isNew;
    time=other.time;
    theme=other.theme;
    text=other.theme;
    email=other.email;
    customer_user_id=other.customer_user_id;
    customer_id=other.customer_user_id;
}

TICKET &TICKET::operator=(const TICKET &other)
{
    ID = other.ID;
    ticket_number = other.ticket_number;
    isNew = other.isNew;
    time=other.time;
    theme=other.theme;
    text=other.theme;
    email=other.email;
    customer_user_id=other.customer_user_id;
    customer_id=other.customer_user_id;
}

bool TICKET::operator==(const TICKET &o) const
{
    return this->ticket_number == o.ticket_number;
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
