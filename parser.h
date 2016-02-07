#ifndef Parser_H
#define Parser_H

#include <QThread>
#include <ticket.h>
#include <QSqlDatabase>

class Parser : public QThread
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    void initialization(QFile *file);
    bool pause;
signals:
    void progress(int);
    void newTicket(TICKET);
protected:
    void run();
private:
    QFile *file;
};

#endif // Parser_H
