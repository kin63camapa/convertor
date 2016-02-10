#include "ticket.h"

TICKET::TICKET()
{
    clear();
}

TICKET::TICKET(const TICKET &other)
{
    this->operator =(other);
}

TICKET &TICKET::operator=(const TICKET &other)
{
    ID = other.ID;
    currenIndex = other.currenIndex;
    injectID = other.injectID;
    ticket_number = other.ticket_number;
    queue_id = other.queue_id;
    user_id=other.user_id;
    isNew = other.isNew;
    creationTime=other.creationTime;
    theme=other.theme;
    text=other.text;
    email=other.email;
    customer_user_id=other.customer_user_id;
    customer_id=other.customer_id;
    messages=other.messages;
    state=other.state;
}

bool TICKET::operator==(const TICKET &o) const
{
    return this->ticket_number == o.ticket_number;
}

void TICKET::clear()
{
    ID = 0;
    injectID = -1;
    ticket_number = 0;
    queue_id = 0;
    isNew = false;
    user_id = 4;//света (office@smart-tech.biz)
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
    else if (!ID && other.ID) this->ID = other.ID;
}

void TICKET::sortMessages()
{
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

void TICKET::removeDuplicates()
{
    QString str;
    QList<Message> tm = messages;
    messages.clear();
    foreach (Message m, tm)
    {
        if (m.text == str || m.text.isEmpty()) continue;
        str=m.text;
        if (str.contains("] в очередь "))
        {
            if (str.contains("Руководство")) queue_id = 4;
            if (str.contains("Руководство::На закрытие")) queue_id = 3;
            if (str.contains("Мастерская")) queue_id = 5;
            if (str.contains("выезды")) queue_id = 6;
            if (str.contains("абонобслуживание")) queue_id = 7;
            if (str.contains("Витязи/Кондраков")) queue_id = 8;
            if (str.contains("Диспетчер")) queue_id = 9;
            if (str.contains("Руководство::на закупку")) queue_id = 10;
            if (str.contains("ожидание оплаты")) queue_id = 11;
            if (str.contains("Чего-то  ждем")) queue_id = 12;
            if (str.contains("абонобслуживание::Кузнецов Иван")) queue_id = 13;
            if (str.contains("Коновалов Дмитрий")) queue_id = /*14*/9;//на диспетчера
            if (str.contains("абонобслуживание::Федоров Александр")) queue_id = /*15*/9;//туда же
            if (str.contains("Мастерская::Дербенев")) queue_id = 16;
            if (str.contains("абонобслуживание::Милешкевич")) queue_id = 17;
            if (str.contains("Диспетчер::выставить счет")) queue_id = 18;
            if (str.contains("Готово к передаче клиенту")) queue_id = 19;
            if (str.contains("Кирилл")) queue_id = 20;
            if (str.contains("Руководство::Сергей")) queue_id = 21;
            if (str.contains("Руководство::Максим")) queue_id = 22;
            if (str.contains("Мастерская::Алексей")) queue_id = 23;
        }
        messages.append(m);
    }
}
