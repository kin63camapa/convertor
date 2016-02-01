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
        TICKET tmpTicket;
        QString tmp = file->readLine();
        while(!file->atEnd())//похоже не отрабатывает
        {//пока файл не кончится
            do
            {//внутри цикла 1 письмо
                if (tmp.contains(QRegExp("^From \\d*@xxx ")))
                {//первая строка мыла, забираем дату+время

                    //QLocale loc(QLocale::English);
                    //tmpTicket.time = loc.toDateTime(tmp.remove(QRegExp("From \\d*@xxx ")).remove(tmp.size()-2,2).replace(" +0000 "," "));
                    //по идее этого достаточно но цука не пашет!!!!!! по этому дальше быдлокот
                    //qDebug() << qPrintable(tmp.remove(QRegExp("From \\d*@xxx ")).remove(tmp.size()-2,2).replace(" +0000 "," "));

                    QString sT = tmp;
                    tmpTicket.time.setTime(QTime::fromString(sT.remove(QRegExp("From \\d*@xxx ... ... .. ")).remove(8,13)));
                    sT = tmp;
                    int mm;
                    sT=sT.remove(QRegExp("From \\d*@xxx ... ")).remove(3,25);
                    if (sT=="Jan") mm=1;
                    if (sT=="Feb") mm=2;
                    if (sT=="Mar") mm=3;
                    if (sT=="Apr") mm=4;
                    if (sT=="May") mm=5;
                    if (sT=="June") mm=6;
                    if (sT=="Jun") mm=6;
                    if (sT=="July") mm=7;
                    if (sT=="Jul") mm=7;
                    if (sT=="Aug") mm=8;
                    if (sT=="Sep") mm=9;
                    if (sT=="Oct") mm=10;
                    if (sT=="Nov") mm=11;
                    if (sT=="Dec") mm=12;
                    sT = tmp;
                    int yy=sT.remove(QRegExp("From \\d*@xxx ... ... ")).remove(1,18).toInt();
                    sT = tmp;
                    int dd=sT.remove(QRegExp("From \\d*@xxx ... ... ")).remove(2,21).toInt();
                    tmpTicket.time.setDate(QDate(yy,mm,dd));
                    qDebug() << tmpTicket.time;
                }
                if (tmp.contains(QRegExp("^Subject: \\[Ticket#\\d*\\] \\[\\d*\\]")))
                {
                    tmpTicket.ticket_number=tmp.remove(QRegExp("Subject: \\[Ticket#")).remove(6,12).toInt();
                    qDebug() << "#" << tmpTicket.ticket_number;
                }
                if (tmp.contains("[1]http://otrs.smart-tech.biz/otrs/index.pl?Action=AgentTicketZoom;Ticket"))
                {
                    tmp = file->readLine();
                    tmp.resize(tmp.size()-2);
                    tmpTicket.ID = tmp.remove(0,3).toInt();
                    qDebug() << "ID" << tmpTicket.ID;
                }
                if (tmp.contains(QRegExp("^Уведомление о новой заявке! \\(.*\\)")))
                {//заявка новая
                    tmpTicket.isNew=true;
                    tmpTicket.theme=QString::fromUtf8(tmp.remove(tmp.size()-3,3).toAscii()).remove(0,29);
                    qDebug() << tmpTicket.theme;
                }
                tmp = file->readLine();
            }while(!tmp.contains(QRegExp("From \\d*@xxx ")));
            //qDebug() << tmp;
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
