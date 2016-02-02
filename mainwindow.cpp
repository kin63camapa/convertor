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

                    qDebug() << QString::fromUtf8(str.remove(QRegExp("[\\n\\t\\r]")).toAscii());
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
        foreach (TICKET t, list)
        {
            qDebug() << t.ID << t.ticket_number << t.time.toString("dd.MM.yyyy hh:mm:ss") << t.theme;

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

void MainWindow::tst()
{
    QSqlQuery q("select login,customer_id from otrs.customer_user where email like \"holod@oootxt.ru\" order by id;",db);
    //QSqlQuery q("select * from otrs.users;",db);
    q.last();
    tab->setRowCount(q.at()+1);
    q.first();
    tab->setColumnCount(q.record().count());
    for (int r = tab->rowCount();r;r--)
    {
        for (int c = tab->columnCount();c;c--)
        {
            QTableWidgetItem *tmp = new QTableWidgetItem(q.value(c-1).toString());
            tab->setItem(r-1,c-1,tmp);
        }
        q.next();
    }
    statusbar->showMessage(q.lastError().text());
    //*/

}
