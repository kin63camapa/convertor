﻿#include "mainwindow.h"
#include "sqlconnectdialog.h"
#include "ticket.h"


MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    qRegisterMetaType<TICKET>("TICKET");
    qRegisterMetaType<FindDialog::FindType>("FindDialog::FindType");
    periodStart = QDateTime::fromString("00:00:00 01.01.2000","hh:mm:ss dd.MM.yyyy");
    periodEnd = QDateTime::fromString("00:00:00 01.01.2100","hh:mm:ss dd.MM.yyyy");
    lastId = -1;
    index = 0;
    lastFindQEry.clear();
    lastFindAnswer = -1;

    parser = new Parser(this);
    fdialog = new FindDialog(this);
    connect(fdialog,SIGNAL(fnd(FindDialog::FindType,QString)),this,SLOT(fnd(FindDialog::FindType,QString)));
    tab = new TicketPreview(this);
    log = new QPlainTextEdit(this);
    connect(tab,SIGNAL(nextBtnClicked()),this,SLOT(nextBtnClicked()));
    connect(tab,SIGNAL(prewBtnClicked()),this,SLOT(prewBtnClicked()));
    connect(tab,SIGNAL(save(TICKET)),this,SLOT(editTicket(TICKET)));
    connect(tab,SIGNAL(inject(TICKET)),this,SLOT(injectTicket(TICKET)));
    statusbar = new QStatusBar(this);
    openMailFolder = new QAction(QString::fromUtf8("Открыть Почту"),this);
    menubar = new QMenuBar(this);
    menu = new QMenu(menubar);
    fmenu = new QMenu(menubar);
    openBase = new QAction(QString::fromUtf8("Открыть Базу"),this);
    find = new QAction(QString::fromUtf8("Найти тикет"),this);
    menu->setTitle("File");
    fmenu->setTitle("Find");
    menu->addAction(openBase);
    menu->addAction(openMailFolder);
    fmenu->addAction(find);
    menubar->addMenu(menu);
    menubar->addMenu(fmenu);
    btn = new QPushButton(this);
    btn->setText(QString::fromUtf8("Сделать все збс"));
    pbar = new QProgressBar(this);
    connect(this->parser,SIGNAL(finished()),this,SLOT(parsetFinished()));
    connect(this->parser,SIGNAL(progress(int)),this,SLOT(pbarinc(int)));
    connect(this->parser,SIGNAL(newTicket(TICKET)),this,SLOT(newTicket(TICKET)));
    connect(openBase,SIGNAL(triggered()),this,SLOT(connectBase()));
    connect(openMailFolder,SIGNAL(triggered()),this,SLOT(openMail()));
    connect(btn,SIGNAL(clicked()),this,SLOT(doAllZBS()));
    connect(find,SIGNAL(triggered()),this,SLOT(findTicket()));
    l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(menubar);
    l->addWidget(log);
    log->hide();
    l->addWidget(tab);
    tab->hide();
    l->addWidget(pbar);
    l->addWidget(btn);
    l->addWidget(statusbar);
    pbar->hide();
    this->setGeometry(this->geometry().x(),this->geometry().y(),500,400);
    find->setDisabled(true);
    connectBase();

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
        unknowEmails.clear();
        parser->initialization(file);
        pbar->setMaximum(file->bytesAvailable());
        pbar->setValue(0);
        pbar->show();
        this->btn->setDisabled(true);
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

void MainWindow::editTicket(TICKET t)
{
    list.replace(t.currenIndex,t);
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

void MainWindow::fnd(FindDialog::FindType t, QString s)
{

    if (s.isEmpty())
    {
        lastFindQEry = s;
        lastFindAnswer = -1;
        if (list.size()) tab->showTicket(list.at(0),list.size());
        QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Задан пустой поисковый запрос!/nСчетчики сброшены. Поиск не выполнялся.")).exec();
        return;
    }
    switch (t)
    {
    case FindDialog::byId:
    {
        int i=0,id=s.toInt();
        foreach (TICKET tmp, list)
        {
            if(tmp.ID == id)
            {
                tab->showTicket(list.at(i),list.size());
                return;
            }
            i++;
        }
        QSqlQuery q(QString("select ticket_state_id from otrs.ticket where id like\"%1\" order by id;").arg(id),db);
        int qLeight;
        q.last();
        qLeight = q.at()+1;//return -1 if req is empty
        q.first();
        if(qLeight==1)
        {
            if (q.value(0).toInt()==4)
            {
                QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!\nПохоже эта заявка ЕСТЬ В БАЗЕ и ЗАКРЫТА!.")).exec();
                return;
            }
        }
        QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!")).exec();
        break;
    }
    case FindDialog::byNum:
    {
        TICKET tmp;
        tmp.ticket_number = s.toInt();
        int i=list.indexOf(tmp);
        if (i==-1)
        {
            getState(&tmp);
            if (tmp.state == TICKET::Closed)
                QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!\nПохоже эта заявка ЕСТЬ В БАЗЕ и ЗАКРЫТА!.")).exec();
            else QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!\nПохоже такого тикета нет.")).exec();

            return;
        }
        tab->showTicket(list.at(i),list.size());
        break;
    }
    case FindDialog::byPos:
    {
        if (s.toInt()>list.size() || s.toInt() < 0)
        {
            QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!\nПохоже вы просите невозможного.")).exec();
            return;
        }
        tab->showTicket(list.at(s.toInt()),list.size());
        break;
    }
    case FindDialog::byStr:
    {
        if (!lastFindQEry.isEmpty())
        {
            if (lastFindQEry != s)
            {
                lastFindQEry = s;
                lastFindAnswer = -1;
            }
        }
        else
        {
            lastFindQEry = s;
            lastFindAnswer = -1;
        }
        int i=0;
        foreach (TICKET tmp, list)
        {
            foreach (TICKET::Message m, tmp.messages)
            {
                if (m.text.contains(s.toLocal8Bit()) && i > lastFindAnswer)
                {
                    tab->showTicket(list.at(i),list.size());
                    lastFindAnswer=i;
                    return;
                }
            }
            i++;
        }
        QString addon;
        if (lastFindAnswer!=-1)
        {
            addon=QString::fromUtf8("\nПри следующем нажатии кнопки поиска он начнется сначала.");
            lastFindAnswer=-1;
        }
        QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!")+addon).exec();
        break;
    }
    default:
    {
        break;
    }
    }
}

void MainWindow::pbarinc(int i)
{
    pbar->setValue(pbar->maximum()-i);
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
        if (!unknowEmails.contains(ret.email))unknowEmails.append(ret.email);
    }
    return ret;
}

TICKET::Status MainWindow::getState(TICKET *t)
{
    QSqlQuery q(QString("select ticket_state_id from otrs.ticket where tn like\"%1\" order by id;").arg(t->ticket_number),db);
    int qLeight;
    q.last();
    qLeight = q.at()+1;//return -1 if req is empty
    q.first();
    if(qLeight==1)
    {
        //есть ощущение, что при значении 4 - заявка закрыта.
        if (q.value(0).toInt()==4) t->state = TICKET::Closed;
        else t->state = TICKET::Exist;
    }
    else if (qLeight==0) t->state = TICKET::New;
    else t->state = TICKET::Unknown;
    return t->state;
}


void MainWindow::newTicket(TICKET t)
{
    if ((t.creationTime < periodStart && !t.isNew) || t.creationTime > periodEnd) return;
    parser->pause = true;
    TICKET::Message m;
    m.time=t.creationTime;
    m.text=t.text;
    t.messages.append(m);
    getUserinfo(&t);
    getState(&t);
    switch (t.state)
    {
    case TICKET::Exist:
        t.injectID=GetId(t);
    case TICKET::New:
    case TICKET::Unknown:
        if (list.contains(t))
        {
            t.currenIndex=list.indexOf(t);
            t.compare(list.at(t.currenIndex));
            t.sortMessages();
            t.removeDuplicates();
            if (t.injectID==-1){t.injectID=lastId+t.currenIndex+1;}
            list.replace(t.currenIndex,t);
        }
        else
        {
            t.currenIndex=list.size();
            if (t.injectID==-1){t.injectID=lastId+t.currenIndex+1;}
            list.append(t);
        }
    case TICKET::Closed:
        break;
    }
    parser->pause = false;
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
    QMessageBox msgBox;
    msgBox.setWindowTitle(QString::fromUtf8("СТОЯТЬ!!!"));
    msgBox.setText(QString::fromUtf8("Программа собирается сделать все збс, но может получится ровно наоборот!\nСобрано %1 тикетов\nИз них с неизвестными e-mail %2\nПодключен к базе %3\n").arg(QString::number(list.size())).arg(QString::number(unknowEmails.size())).arg(db.hostName()+":"+QString::number(db.port())));
    msgBox.setInformativeText(QString::fromUtf8("Продолжаем?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.button(QMessageBox::Save)->setText(QString::fromUtf8("Автоматически"));
    msgBox.button(QMessageBox::Discard)->setText(QString::fromUtf8("Вручную"));
    msgBox.button(QMessageBox::Cancel)->setText(QString::fromUtf8("Не-не-не, Девид Блейн, раскукож меня обратно!"));
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
        // auto
        log->show();
        find->setDisabled(true);
        fdialog->hide();
        foreach (TICKET t, list)
        {
            inject(t);
        }
        break;
    case QMessageBox::Discard:
        // manual
        log->hide();
        index=0;
        tab->show();
        find->setDisabled(false);
        tab->showTicket(list.at(0),list.size());
        break;
    case QMessageBox::Cancel:
    default:
        return;
        break;
    }
}

void MainWindow::parsetFinished()
{
    pbar->hide();
    btn->setEnabled(true);
    if (unknowEmails.size())
    {
        log->textCursor().insertText(QString::fromUtf8("Список неизвестных адресов:\n"));
        foreach (QString s, unknowEmails)
        {
            log->textCursor().insertText(s+"\n");
        }
        log->textCursor().insertText(QString::fromUtf8("Конец списка нейзвестных адресов.\n"));
        log->show();
    }

}

void MainWindow::injectTicket(TICKET t)
{
    if (QMessageBox(QMessageBox::Warning,
                    QString::fromUtf8("СТОЯТЬ!!!"),
                    QString::fromUtf8("Вы собираетесь записать тикет в базу!\nОтменить это действие невозможно!\nПродолжать?"),
                    QMessageBox::Yes|QMessageBox::No).exec() != QMessageBox::Yes) return;
    inject(t);
}

void MainWindow::findTicket()
{
    fdialog->show();
}


bool MainWindow::inject(TICKET t)
{
    ;
}
