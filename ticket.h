#ifndef TICKET_H
#define TICKET_H

#include <QtCore>

class TICKET : public QObject
{
    Q_OBJECT
public:
    explicit TICKET(QObject *parent = 0);
signals:

public slots:

private:
    int ID;
    QDateTime time;
    QString theme;
    QString text;
    QString email;
    QString customer_user_id;
    QString customer_id;
};

#endif // TICKET_H
