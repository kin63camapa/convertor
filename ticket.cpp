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
    creationTime=other.creationTime;
    theme=other.theme;
    text=other.theme;
    email=other.email;
    customer_user_id=other.customer_user_id;
    customer_id=other.customer_user_id;
    messages=other.messages;
}

TICKET &TICKET::operator=(const TICKET &other)
{
    ID = other.ID;
    ticket_number = other.ticket_number;
    isNew = other.isNew;
    creationTime=other.creationTime;
    theme=other.theme;
    text=other.theme;
    email=other.email;
    customer_user_id=other.customer_user_id;
    customer_id=other.customer_user_id;
    messages=other.messages;
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
    creationTime=clearDT;
    theme.clear();
    text.clear();
    email.clear();
    customer_user_id.clear();
    customer_id.clear();
    messages.clear();
}

void TICKET::compare(TICKET other)
{
    messages+=other.messages;
    other.messages=messages;
    if (other.isNew)this->operator =(other);
}

void TICKET::sortMessages()
{
    message tmp;
    for(int i = 0; i < messages.size() - 1; ++i)
    {
        for(int j = 0; j < messages.size() - 1; ++j)
        {
            if (messages.at(j + 1).time < messages.at(j).time)
            {
                messages.swap(j,j+1);
            }
        }
    }
}
