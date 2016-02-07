#include <QSqlQuery>
#include "parser.h"

Parser::Parser(QObject *parent) :
    QThread(parent)
{
}

void Parser::initialization(QFile *file)
{
    this->file=file;
}

void Parser::run()
{
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
                        tmp.clear();

                    }
                    bufftx.append(tmp);
                }
                while(!bufftx.at(bufftx.size()-1).contains("http://otrs.smart-tech.biz/otrs/index.pl?Action=AgentTicketZoom;Ticket"));
                tmp = file->readLine();
                emit progress(file->bytesAvailable());
                tmp.resize(tmp.size()-2);
                id=tmpTicket.ID=tmp.remove(0,3).toInt();
                tmp.clear();
                bool rmSpaces=true;
                foreach (QString tmptx, bufftx)
                {
                    if (
                            tmptx.contains("Content-Disposition:")||
                            tmptx.contains("Content-Transfer-Encoding:")||
                            tmptx.contains("[1]http://otrs.smart-tech.biz")||
                            tmptx.isEmpty()
                            )continue;
                    if (rmSpaces)
                    {
                        tmptx.remove(QRegExp("[\\n\\t\\r]"));
                        if (tmptx.contains("Заметка:"))
                        {
                            rmSpaces=false;
                        }
                    }else
                    {
                        tmptx.replace(QRegExp("[\\n\\t\\r]")," ");
                    }
                    tmp += tmptx;
                }
                if (!tmpTicket.isNew)
                {
                    tmp.remove(QRegExp("<snip>.*<snip>"));
                }
                else
                {
                    tmp.replace("<snip>","");
                    tmp.replace("> ","\n");
                }

                tmp.replace(".Заметка:",".\nЗаметка: ");
                tmp.replace("Назакрытие","На закрытие");
                tmp.replace(QRegExp("Привет [\\S]*\\,")," ");
                tmp.replace(QRegExp("Moved ticket in .*\\) "),"");

                tmpTicket.text=tmp+"\n";
                text=tmp.size();
                //qDebug() << QString::fromUtf8(tmpTicket.text.toAscii());
            }
            tmp = file->readLine();
            emit progress(file->bytesAvailable());
        }while(file->bytesAvailable()&&!tmp.contains(QRegExp("From \\d*@xxx ")));
        //тут тикет по результатам чтения письма
        emit newTicket(tmpTicket);
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
}
