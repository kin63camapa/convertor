#ifndef TICKET_H
#define TICKET_H

#include <QtCore>

struct Userinfo
{
    bool isOk;
    QString customer_user_id;
    QString customer_id;
};

class TICKET : public QObject
{
    Q_OBJECT
public:
    TICKET();               // конструктор по умолчанию
    TICKET(const TICKET &other);  // конструктор копирования
    TICKET &operator=(const TICKET &other);  // оператор присваивания
    bool operator==(const TICKET& o)const;
    void clear();


    int ID;//
    int ticket_number;//
    QDateTime time;//
    QString theme;
    QString text;
    QString email;
    QString customer_user_id;
    QString customer_id;
    bool isNew;//

signals:

public slots:

private:

};

extern QList<TICKET> list;
#endif // TICKET_H
