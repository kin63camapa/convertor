#include <QSqlQuery>
#include "parser.h"

Parser::Parser(QObject *parent) :
    QThread(parent)
{
}

void Parser::initialization(QStringList *unknowEmails, QFile *file, QSqlDatabase *db)
{
    this->unknowEmails=unknowEmails;
    //this->list=list;
    this->file=file;
    this->db=db;
}

void Parser::run()
{
    list.clear();
    unknowEmails->clear();
    bool id=false;
    bool num=false;
    bool time=false;
    bool theme=false;
    bool text=false;
    bool email=false;
    bool customer_user_id=false;
    bool customer_id=false;
    bool isNew=false;
    bool htmlIsPresent=false;
    bool plainIsPresent=false;
    QString tmp = file->readLine();
    emit progress(file->bytesAvailable());
    TICKET tmpTicket;
    while(file->bytesAvailable())
    {//пока файл не кончится
        do
        {//внутри цикла 1 письмо
            if (!time && tmp.contains(QRegExp("^From \\d*@xxx ")))
            {//первая строка мыла, забираем дату+время
                QLocale loc(QLocale::English);
                tmpTicket.creationTime = loc.toDateTime(
                            tmp.remove(QRegExp("From \\d*@xxx ")).remove(30,100).replace(" +0000 "," "),"ddd MMM dd hh:mm:ss yyyy");
                tmpTicket.creationTime=QDateTime::fromTime_t(tmpTicket.creationTime.toTime_t()+60*60*4);
                time=tmpTicket.creationTime.isValid();
                if (time) time=!tmpTicket.creationTime.isNull();
            }
            //if (!num&&tmp.contains(QRegExp("^Subject: \\[Ticket#\\d*\\] \\[\\d*\\]")))
            if (!num&&tmp.contains(QRegExp("^Subject: \\[Ticket#\\d*\\]")))
            {//получаем номер
                QString tmp1 = tmp;
                num=tmpTicket.ticket_number=tmp.remove(QRegExp("Subject: \\[Ticket#")).remove(6,12).toInt();//индус думает что длинна номера всегда 6 символов
                if (!tmpTicket.ticket_number)
                {
                    tmp1 = tmp1.remove(QRegExp("Subject: \\[Ticket#"));
                    tmp1 = tmp1.remove(6,tmp1.size()-6);
                    num=tmpTicket.ticket_number=tmp1.toInt();
                }
            }
            if (!text && tmp.contains(QRegExp("Content-Type: text/plain; charset\"utf-8\"")))
            {
                QStringList bufftx;
                do
                {
                    tmp=file->readLine();
                    emit progress(file->bytesAvailable());
                    if (!theme && !tmpTicket.isNew && tmp.contains(QRegExp("^Уведомление о новой заявке! \\(.*\\)")))
                    {//заявка новая
                        tmpTicket.isNew=true;
                        tmpTicket.theme=QString::fromUtf8(tmp.remove(tmp.size()-3,3).toAscii()).remove(0,29);
                        theme=tmpTicket.theme.size();
                        QStringList buff;
                        do
                        {
                            buff.append(file->readLine());
                            emit progress(file->bytesAvailable());
                        }
                        while (!buff.at(buff.size()-1).contains("сообщает:")||buff.size()==10);

                        QString str=
                                buff.at(buff.size()-4)
                                +buff.at(buff.size()-3)
                                +buff.at(buff.size()-2)
                                +buff.at(buff.size()-1);

                        str.remove(QRegExp("[\\n\\t\\r]"));
                        str.remove(QRegExp("([А-я0-9!\"].*\"\ ?<)"));
                        // Чистим строку до символа '<' Также чукча не знает, есть ли ветвление в регулярках отсюда нижняя строка
                        str.remove(QRegExp("([А-я!].*\\x00A0(?=[0-9A-z]))"));
                        str.remove(">, сообщает:");
                        str.remove(">,сообщает:");
                        str.remove(", сообщает:");
                        str.remove(",сообщает:");
                        tmpTicket.email=str.remove(QRegExp("([А-я].*\\x00A0(?=[A-z0-9]))"));
                        // Должен удалять Non-breaking space(или U+00A0)
                        Userinfo nfo = getFromEmail(tmpTicket.email);
                        if (nfo.isOk)
                        {
                            tmpTicket.customer_id = nfo.customer_id;
                            tmpTicket.customer_user_id = nfo.customer_user_id;
                            //qDebug() << tmpTicket.email << tmpTicket.customer_user_id << tmpTicket.customer_id;
                        }
                        else
                        {
                            if (!unknowEmails->contains(tmpTicket.email))unknowEmails->append(tmpTicket.email);
                            //qDebug() << "can not find user or conpany by emails " << tmpTicket.email;
                        }

                    }
                    bufftx.append(tmp);
                }
                while(!bufftx.at(bufftx.size()-1).contains("[1]http://otrs.smart-tech.biz/otrs/index.pl?Action=AgentTicketZoom;Ticket"));
                if (!id)
                {//получаем id старой базы
                    tmp = file->readLine();
                    emit progress(file->bytesAvailable());
                    tmp.resize(tmp.size()-2);
                    id=tmpTicket.ID=tmp.remove(0,3).toInt();
                }
                foreach (QString tmptx, bufftx)
                {
                    if (
                            tmptx.contains("Content-Disposition:")||
                            tmptx.contains("Content-Transfer-Encoding:")||
                            tmptx.contains("[1]http://otrs.smart-tech.biz"
                                           ))continue;
                    tmp += tmptx;
                }
                tmpTicket.text=tmp;
                text=tmp.size();
                //qDebug() << QString::fromUtf8(tmp.toAscii());
            }
            tmp = file->readLine();
            emit progress(file->bytesAvailable());
        }while(file->bytesAvailable()&&!tmp.contains(QRegExp("From \\d*@xxx ")));
        //тут тикет по результатам чтения письма
        TICKET::message m;
        m.time=tmpTicket.creationTime;
        m.text=tmpTicket.text;
        tmpTicket.messages.append(m);
        if (list.contains(tmpTicket))
        {
            tmpTicket.compare(list.at(list.indexOf(tmpTicket)));
            list.removeAt(list.indexOf(tmpTicket));
            list.append(tmpTicket);
        }
        else
        {
            list.append(tmpTicket);
        }
        //прибераемся
        tmpTicket.clear();
        id=false;
        num=false;
        time=false;
        theme=false;
        text=false;
        email=false;
        customer_user_id=false;
        customer_id=false;
        isNew=false;
        htmlIsPresent=false;
        plainIsPresent=false;
    }
    //закончили парсить файл
    qDebug() << *unknowEmails;
    foreach (TICKET t, list)
    {
        qDebug() << t.ID << t.ticket_number << t.creationTime.toString("dd.MM.yyyy hh:mm:ss") << t.theme << t.email<<t.customer_user_id<<t.customer_id;
        foreach (TICKET::message m, t.messages)
        {
            //qDebug() << m.time.toString("dd.MM.yyyy hh:mm:ss") << QString::fromUtf8(m.text.toAscii());
        }

    }
}


Userinfo Parser::getFromEmail(QString email)
{
    Userinfo userinfo;
    userinfo.isOk=false;
    QSqlQuery q(QString("select login,customer_id from otrs.customer_user where email like \"%1\" order by id;").arg(email),*db);
    //    statusbar->showMessage(q.lastError().text());
    int qLeight;
    q.last();
    qLeight = q.at()+1;//return -1 if req is empty
    q.first();
    if(qLeight==1)
    {
        userinfo.customer_user_id=q.value(0).toString();
        userinfo.customer_id=q.value(1).toString();
        userinfo.isOk=(!userinfo.customer_id.isEmpty())&&(!userinfo.customer_user_id.isEmpty());
    }

    return userinfo;
}
