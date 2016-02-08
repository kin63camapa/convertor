#ifndef TICKET_H
#define TICKET_H

#include <QtCore>

class TICKET : public QObject
{
    Q_OBJECT
public:
    TICKET();               // конструктор по умолчанию
    TICKET(const TICKET &other);  // конструктор копирования
    TICKET &operator=(const TICKET &other);  // оператор присваивания
    bool operator==(const TICKET& o)const;
    void clear();
    void compare(TICKET other);
    void sortMessages();
    void removeDuplicates();
    enum Status
    {
        Closed,
        Exist,
        New,
        Unknown
    } state;
    struct Message
    {
        QDateTime time;//
        QString text;
    };
    int ID;//
    int injectID;
    int ticket_number;//
    int currenIndex;
    int queue_id;
    int user_id;
    QDateTime creationTime;//
    QString theme;
    QString text;
    QString email;
    QString customer_user_id;
    QString customer_id;
    bool isNew;//
    QList<Message> messages;
};

#endif // TICKET_H
