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
    void initialization(QStringList *unknowEmails,QFile *file,QSqlDatabase *db);
signals:
    void progress(int);
protected:
    void run();
private:
    QStringList *unknowEmails;
    QFile *file;
    QSqlDatabase *db;
    Userinfo getFromEmail(QString email);
};

#endif // Parser_H
