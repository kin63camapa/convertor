#include "mainwindow.h"
#include "sqlconnectdialog.h"
#include "ticket.h"

QStringList loadFiles(QDir startDir, QStringList filters)
{

    QStringList list;

    foreach (QString file, startDir.entryList(filters, QDir::Files))
        list += QFileInfo(startDir.absolutePath() + QDir::separator() + file).absoluteFilePath();

    foreach (QString subdir, startDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        list += loadFiles(QDir(startDir.absolutePath() + QDir::separator() + subdir), filters);
    return list;
}

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    unknowEmails = NULL;
    tab = new QTableWidget(this);
    statusbar = new QStatusBar(this);
    openMailFolder = new QAction(QString::fromUtf8("Открыть Почту"),this);
    menubar = new QMenuBar(this);
    menu = new QMenu(menubar);
    openBase = new QAction(QString::fromUtf8("Открыть Базу"),this);
    menu->setTitle("File");
    menu->addAction(openBase);
    menu->addAction(openMailFolder);
    menubar->addMenu(menu);
    connect(openBase,SIGNAL(triggered()),this,SLOT(connectBase()));
    connect(openMailFolder,SIGNAL(triggered()),this,SLOT(openMail()));
    btn = new QPushButton(this);
    btn->setText(QString::fromUtf8("Сделать все збс"));
    connect(btn,SIGNAL(clicked()),this,SLOT(tst()));
    l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(menubar);
    l->addWidget(tab);
    l->addWidget(btn);
    l->addWidget(statusbar);
    //connectBase();

}

void MainWindow::openMail()
{

    QFile *file = new QFile(QFileDialog::getOpenFileName(this));
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Не удалось открыть файл: %1").arg(file->fileName())).exec();
    }
    else
    {//открыли файл
        QList<TICKET> list;
        if (unknowEmails) delete unknowEmails;
        unknowEmails = new QStringList();
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
        TICKET tmpTicket;
        QString tmp = file->readLine();
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

                if (!plainIsPresent && tmp.contains("Content-Type: text/plain; charset\"utf-8\""))
                {
                    plainIsPresent=true;
                }
                if (!htmlIsPresent && tmp.contains("Content-Type: text/plain; charset\"utf-8\""))
                {
                    htmlIsPresent=true;
                }
                if (htmlIsPresent!=plainIsPresent) QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("coder is invalid")).exec();


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
}

void MainWindow::connectBase()
{
    SQLConnectDialog openDialog;
    db = openDialog.connect(db);
    if (!db.isOpen())
    {
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Подключения к базе нет, работаем в режиме парсера.")).exec();
    }
}

Userinfo MainWindow::getFromEmail(QString email)
{
    Userinfo userinfo;
    userinfo.isOk=false;
    QSqlQuery q(QString("select login,customer_id from otrs.customer_user where email like \"%1\" order by id;").arg(email),db);
    //    QSqlQuery q("select * from otrs.users;",db);
    //    q.last();
    //    tab->setRowCount(q.at()+1);
    //    q.first();
    //    tab->setColumnCount(q.record().count());
    //    for (int r = tab->rowCount();r;r--)
    //    {
    //        for (int c = tab->columnCount();c;c--)
    //        {
    //            QTableWidgetItem *tmp = new QTableWidgetItem(q.value(c-1).toString());
    //            tab->setItem(r-1,c-1,tmp);
    //        }
    //        q.next();
    //    }
    int qLeight;
    q.last();
    qLeight = q.at()+1;//return -1 if req is empty
    q.first();
    if(qLeight==1)
    {
        userinfo.customer_id=q.value(0).toString();
        userinfo.customer_user_id=q.value(1).toString();
        userinfo.isOk=(!userinfo.customer_id.isEmpty())&&(!userinfo.customer_user_id.isEmpty());
    }
    statusbar->showMessage(q.lastError().text());
    return userinfo;
}
