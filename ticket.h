#ifndef TICKET_H
#define TICKET_H

#include <QtCore>

class TICKET : public QObject
{
    Q_OBJECT
public:
    explicit TICKET(QObject *parent = 0);
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

#endif // TICKET_H
