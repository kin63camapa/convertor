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
                tmpTicket.time = loc.toDateTime(
                            tmp.remove(QRegExp("From \\d*@xxx ")).remove(30,100).replace(" +0000 "," "),"ddd MMM dd hh:mm:ss yyyy");
                tmpTicket.time=QDateTime::fromTime_t(tmpTicket.time.toTime_t()+60*60*4);
                time=tmpTicket.time.isValid();
                if (time) time=!tmpTicket.time.isNull();
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
            if (!id&&tmp.contains("[1]http://otrs.smart-tech.biz/otrs/index.pl?Action=AgentTicketZoom;Ticket"))
            {//получаем id старой базы
                tmp = file->readLine();
                emit progress(file->bytesAvailable());
                tmp.resize(tmp.size()-2);
                id=tmpTicket.ID=tmp.remove(0,3).toInt();
            }
            if (!theme && !tmpTicket.isNew && tmp.contains(QRegExp("^Уведомление о новой заявке! \\(.*\\)")))
            {//заявка новая
                tmpTicket.isNew=true;
                tmpTicket.theme=QString::fromUtf8(tmp.remove(tmp.size()-3,3).toAscii()).remove(0,29);
                theme=tmpTicket.theme.size();
                //тут надо получить мыло, и, по возможности, customer_id и customer_user_id
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
                    qDebug() << tmpTicket.email << tmpTicket.customer_user_id << tmpTicket.customer_id;
                }
                else
                {
                    if (!unknowEmails->contains(tmpTicket.email))unknowEmails->append(tmpTicket.email);
                    //qDebug() << "can not find user or conpany by emails " << tmpTicket.email;
                }
            }
            if (!text && tmp.contains(QRegExp("<snip>")))
            {
                QStringList bufftx;
                do
                {

                    bufftx.append(file->readLine());
                }
                while(!bufftx.at(bufftx.size()-1).contains(QRegExp("<snip>")));
                QString strtx;
                strtx = bufftx.at(bufftx.size()-4)+
                        bufftx.at(bufftx.size()-3)+
                        bufftx.at(bufftx.size()-2)+
                        bufftx.at(bufftx.size()-1);
                qDebug() << QString::fromUtf8(strtx.toAscii());
            }

            if (!plainIsPresent && tmp.contains("Content-Type: text/plain; charset\"utf-8\""))
            {
                plainIsPresent=true;
            }
            if (!htmlIsPresent && tmp.contains("Content-Type: text/plain; charset\"utf-8\""))
            {
                htmlIsPresent=true;
            }
            //if (htmlIsPresent!=plainIsPresent) QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("coder is invalid")).exec();


            tmp = file->readLine();
        }while(file->bytesAvailable()&&!tmp.contains(QRegExp("From \\d*@xxx ")));
        //тут тикет по результатам чтения письма

        if (!list.contains(tmpTicket))
        {
            //qDebug() << "add new ticket" << tmpTicket.ticket_number;
            list.append(tmpTicket);
        }
        else
        {
            if (list.at(list.indexOf(tmpTicket)).time>tmpTicket.time);
            //qDebug() << "find additional for" << list.at(list.indexOf(tmpTicket)).ticket_number;

            if (list.at(list.indexOf(tmpTicket)).ticket_number!=tmpTicket.ticket_number);

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
        //qDebug() << t.ID << t.ticket_number << t.time.toString("dd.MM.yyyy hh:mm:ss") << t.theme;

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
