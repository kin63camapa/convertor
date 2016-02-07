#include "mainwindow.h"
#include "sqlconnectdialog.h"
#include "ticket.h"


MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    qRegisterMetaType<TICKET>("TICKET");
    periodStart = QDateTime::fromString("00:00:00 01.01.2000","hh:mm:ss dd.MM.yyyy");
    periodEnd = QDateTime::fromString("00:00:00 01.01.2100","hh:mm:ss dd.MM.yyyy");
    lastId = -1;
    index = 0;
    parser = new Parser(this);
    unknowEmails = new QStringList();
    tab = new TicketPreview(this);
    connect(tab,SIGNAL(nextBtnClicked()),this,SLOT(nextBtnClicked()));
    connect(tab,SIGNAL(prewBtnClicked()),this,SLOT(prewBtnClicked()));
    connect(tab,SIGNAL(save(TICKET)),this,SLOT(editTicket(TICKET)));
    statusbar = new QStatusBar(this);
    openMailFolder = new QAction(QString::fromUtf8("Открыть Почту"),this);
    menubar = new QMenuBar(this);
    menu = new QMenu(menubar);
    openBase = new QAction(QString::fromUtf8("Открыть Базу"),this);
    menu->setTitle("File");
    menu->addAction(openBase);
    menu->addAction(openMailFolder);
    menubar->addMenu(menu);
    pbar = new QProgressBar(this);
    connect(this->parser,SIGNAL(finished()),this->pbar,SLOT(hide()));
    connect(this->parser,SIGNAL(progress(int)),this,SLOT(pbarinc(int)));
    connect(this->parser,SIGNAL(newTicket(TICKET)),this,SLOT(newTicket(TICKET)));
    connect(openBase,SIGNAL(triggered()),this,SLOT(connectBase()));
    connect(openMailFolder,SIGNAL(triggered()),this,SLOT(openMail()));
    btn = new QPushButton(this);
    btn->setText(QString::fromUtf8("Сделать все збс"));
    connect(btn,SIGNAL(clicked()),this,SLOT(doAllZBS()));
    l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(menubar);
    l->addWidget(tab);
    tab->hide();
    l->addWidget(pbar);
    l->addWidget(btn);
    l->addWidget(statusbar);
    pbar->hide();
    this->setGeometry(this->geometry().x(),this->geometry().y(),500,400);
    connectBase();

}

void MainWindow::editTicket(TICKET t)
{
    list.replace(t.currenIndex,t);
}

void MainWindow::openMail()
{
    if (!db.isOpen())
    {
        if (QMessageBox(QMessageBox::Warning,
                        QString::fromUtf8("Апшипка"),
                        QString::fromUtf8("Нет подключения к дазе банных! Настоятельно не рекомендуется продолжать в таком режиме!\nДля подключения откройте меню \"File\"->\"Открыть Базу\""),
                        QMessageBox::Ignore|QMessageBox::Ok).exec() != QMessageBox::Ignore) return;
    }
    QFile *file = new QFile(QFileDialog::getOpenFileName(this));
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Не удалось открыть файл: %1").arg(file->fileName())).exec();
    }
    else
    {
        list.clear();
        unknowEmails->clear();
        parser->initialization(file);
        pbar->setMaximum(file->bytesAvailable());
        pbar->setValue(0);
        pbar->show();
        parser->start();
    }
}

void MainWindow::connectBase()
{
    SQLConnectDialog openDialog;
    db = openDialog.connect(db);
    if (!db.isOpen())
    {
        lastId = -1;
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Подключение к базе не выполнено!")).exec();
    }else
    {
        QSqlQuery q(QString("select max(id) from otrs.ticket;"),db);
        int qLeight;
        q.last();
        qLeight = q.at()+1;//return -1 if req is empty
        q.first();
        if(qLeight==1)
        {
            lastId = q.value(0).toInt();

        }else
        {
            lastId = -1;
        }
        statusbar->showMessage("Connected:"+db.hostName()+":"+QString::number(db.port())+" by name "+db.connectionName()+". Last ricket id is "+QString::number(lastId));
    }
}

TICKET MainWindow::getUserinfo(TICKET *t)
{
    TICKET ret;
    QSqlQuery q(QString("select login,customer_id from otrs.customer_user where email like \"%1\" order by id;").arg(t->email),db);
    //    statusbar->showMessage(q.lastError().text());
    int qLeight;
    q.last();
    qLeight = q.at()+1;//return -1 if req is empty
    q.first();
    if(qLeight==1)
    {
        t->customer_user_id=q.value(0).toString();
        t->customer_id=q.value(1).toString();
        ret = *t;
    }
    else
    {
        ret = *t;
        if (!unknowEmails->contains(ret.email))unknowEmails->append(ret.email);
    }
    return ret;
}


void MainWindow::nextBtnClicked()
{
    index++;
    tab->showTicket(list.at(index),list.size());
}

void MainWindow::prewBtnClicked()
{
    index--;
    tab->showTicket(list.at(index),list.size());
}

void MainWindow::doAllZBS()
{
    if (!db.isOpen())
    {
        QMessageBox(QMessageBox::Critical,QString::fromUtf8("Апшипка"),QString::fromUtf8("Нет подключения к дазе банных!\nОткройте меню \"File\"->\"Открыть Базу\"")).exec();
        return;
    }
    if (list.isEmpty())
    {
        QMessageBox(QMessageBox::Critical,QString::fromUtf8("Апшипка"),QString::fromUtf8("Не прочитан файл почты или возникли ошибки при его обработке.\nОткройте меню \"File\"->\"Открыть Почу\"")).exec();
        return;
    }

    index=0;
    tab->show();
    tab->showTicket(list.at(0),list.size());
////    getUserinfo(&t);
////    getState(&t);

//    QMessageBox msgBox;
//    msgBox.setWindowTitle(QString::fromUtf8("СТОЯТЬ!!!"));
//    msgBox.setText(QString::fromUtf8("Программа собирается сделать все збс, но может получится ровно наоборот!\nСобрано %1 тикетов\nИз них с неизвестными e-mail %2\nПодключен к базе %3\n").arg(QString::number(list.size())).arg(QString::number(unknowEmails->size())).arg(db.hostName()+":"+QString::number(db.port())));
//    msgBox.setInformativeText(QString::fromUtf8("Продолжаем?"));
//    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//    msgBox.button(QMessageBox::Save)->setText(QString::fromUtf8("Автоматически"));
//    msgBox.button(QMessageBox::Discard)->setText(QString::fromUtf8("Вручную"));
//    msgBox.button(QMessageBox::Cancel)->setText(QString::fromUtf8("Не-не-не, Девид Блейн, раскукож меня обратно!"));
//    msgBox.setDefaultButton(QMessageBox::Cancel);
//    int ret = msgBox.exec();
//    switch (ret) {
//    case QMessageBox::Save:
//        // auto
//        break;
//    case QMessageBox::Discard:
//        // manual
//        break;
//    case QMessageBox::Cancel:
//    default:
//        return;
//        break;
//    }
}

void MainWindow::pbarinc(int i)
{
    pbar->setValue(pbar->maximum()-i);
}

void MainWindow::newTicket(TICKET t)
{
    if ((t.creationTime < periodStart && !t.isNew) || t.creationTime > periodEnd) return;
    TICKET::Message m;
    m.time=t.creationTime;
    m.text=t.text;
    t.messages.append(m);
    getUserinfo(&t);
    if (list.contains(t))
    {
        t.currenIndex=list.indexOf(t);
        t.compare(list.at(t.currenIndex));
        t.sortMessages();
        list.replace(t.currenIndex,t);
    }
    else
    {
        t.currenIndex=list.size();
        list.append(t);
    }
}

int MainWindow::GetId(TICKET t)
{
    QSqlQuery q(QString("select id from otrs.ticket where tn like \"%1\" order by id;").arg(t.ticket_number),db);
    int qLeight;
    q.last();
    qLeight = q.at()+1;//return -1 if req is empty
    q.first();
    if(qLeight==1)
    {
        return q.value(0).toInt();

    }else
    {
        return -1;
    }
    //QSqlQuery q(QString("select ticket_state_id from otrs.ticket where tn like\"%1\" order by id;").arg(i),*db);
    //есть ощущение, что при значении 4 - заявка закрыта.
}

bool MainWindow::inject(TICKET t)
{
    ;
}
