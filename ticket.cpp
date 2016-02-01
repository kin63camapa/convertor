#include "ticket.h"

TICKET::TICKET(QObject *parent) :
    QObject(parent)
{
    ID = 0;
    ticket_number = 0;
    isNew = false;
}
