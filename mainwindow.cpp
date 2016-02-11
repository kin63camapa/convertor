#include "mainwindow.h"
#include "sqlconnectdialog.h"
#include "ticket.h"


MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    qRegisterMetaType<TICKET>("TICKET");
    qRegisterMetaType<FindDialog::FindType>("FindDialog::FindType");
    periodStart = QDateTime::fromString("00:00:00 28.04.2015","hh:mm:ss dd.MM.yyyy");
    periodEnd = QDateTime::fromString("00:00:00 10.01.2016","hh:mm:ss dd.MM.yyyy");
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
    menubar = new QMenuBar();
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

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (QMessageBox(QMessageBox::Warning,
                    QString::fromUtf8("СТОЯТЬ!!!"),
                    QString::fromUtf8("Выйти?"),
                    QMessageBox::Yes|QMessageBox::No).exec() == QMessageBox::Yes)
    {
        db.close();
        e->accept();
    }
    else e->ignore();
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
            if (q.value(0).toInt()==2||q.value(0).toInt()==3||q.value(0).toInt()==7)
            {
                QMessageBox(QMessageBox::Information,QString::fromUtf8("Ой"),QString::fromUtf8("Ничего не найдено!\nПохоже эта заявка ЕСТЬ В БАЗЕ и ЗАКРЫТА или объеденена!.")).exec();
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

        if (q.value(0).toInt()==2||q.value(0).toInt()==3||q.value(0).toInt()==7) t->state = TICKET::Closed;
        else t->state = TICKET::Exist;
    }
    else if (qLeight==0) t->state = TICKET::New;
    else t->state = TICKET::Unknown;
    return t->state;
}


void MainWindow::newTicket(TICKET t)
{
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
            if (inject(t)) log->textCursor().insertText(QString::fromUtf8("Тикет №%1 записан в базу успешно.\n").arg(t.ticket_number));
            else log->textCursor().insertText(QString::fromUtf8("Тикет №%1 записать в базу Не удалось.\n%2").arg(t.ticket_number).arg(db.lastError().text()));
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
    if (inject(t)) log->textCursor().insertText(QString::fromUtf8("Тикет №%1 записан в базу успешно.\n").arg(t.ticket_number));
    else log->textCursor().insertText(QString::fromUtf8("Тикет №%1 записать в базу Не удалось.\n%2").arg(t.ticket_number).arg(db.lastError().text()));
}

void MainWindow::findTicket()
{
    fdialog->show();
}


bool MainWindow::inject(TICKET t)
{
    if (t.ID <= 0 || t.ticket_number <= 0 || t.injectID <= 0) return false;
    if (t.creationTime > periodEnd)
    {
        log->textCursor().insertText(QString::fromUtf8("Тикет №%1 не записан в базу.\nДата создания %2 позднее %3\n").arg(t.ticket_number).arg(t.creationTime.toString()).arg(periodEnd.toString()));
        return false;
    }
    /*if (t.creationTime < periodStart)
    {
         foreach (TICKET::Message m, t.messages) {
            if (m.time)
        }
        log->textCursor().insertText(QString::fromUtf8("Тикет №%1 записаnm в базу.Дата создания %2 ранее %3").arg(t.ticket_number).arg(t.creationTime).arg(periodStart));
        return;
    }*/
    switch (t.state)
    {
    case TICKET::New:
    {
        QString ticketHeader = QString("INSERT INTO otrs.ticket (tn,title,queue_id,ticket_lock_id,type_id,user_id,responsible_user_id,ticket_priority_id,ticket_state_id,customer_id,customer_user_id,timeout,until_time,escalation_time,escalation_update_time,escalation_response_time,escalation_solution_time,valid_id,archive_flag,create_time_unix,create_time,create_by,change_time,change_by) ");
        QString ticketValues = QString("VALUES (<{tn}>,<{title}>,<{queue_id}>,1,1,<{user_id}>,<{responsible_user_id}>,3,<{ticket_state_id}>,<{customer_id}>,<{customer_user_id}>,0,0,0,0,0,0,1,0,<{create_time_unix}>,<{create_time}>,4,<{change_time}>,4);");
        ticketValues.replace("<{tn}>",QString::number(t.ticket_number));
        ticketValues.replace("<{title}>","\""+t.theme+"\"");
       /* if (t.queue_id < 3 || t.queue_id > 23)*/ t.queue_id = 14;
        ticketValues.replace("<{queue_id}>",QString::number(t.queue_id));
        ticketValues.replace("<{user_id}>",QString::number(t.user_id));
        ticketValues.replace("<{responsible_user_id}>",QString::number(t.user_id));
        ticketValues.replace("<{ticket_state_id}>",QString::number(4));
        if (!t.customer_id.size() || !t.customer_user_id.size())
        {
            t.customer_id = QString::fromUtf8("СмартТех");
            t.customer_user_id = QString::fromUtf8("Кондраков Максим");
        }
        ticketValues.replace("<{customer_id}>","\""+t.customer_id+"\"");
        ticketValues.replace("<{customer_user_id}>","\""+t.customer_user_id+"\"");
        ticketValues.replace("<{create_time_unix}>",QString::number(t.creationTime.toTime_t()));
        ticketValues.replace("<{create_time}>",t.creationTime.toString("\"yyyy-MM-dd hh:mm:ss\""));
        if (t.messages.size())
            ticketValues.replace("<{change_time}>",t.messages.at(t.messages.size()-1).time.toString("\"yyyy-MM-dd hh:mm:ss\""));
        else ticketValues.replace("<{change_time}>",t.creationTime.toString("\"yyyy-MM-dd hh:mm:ss\""));
        QSqlQuery ticketInj(ticketHeader+ticketValues,db);
        if (ticketInj.lastError().type() != QSqlError::NoError)
        {
            qDebug() << ticketHeader+ticketValues << ticketInj.lastError().text();
            log->textCursor().insertText(ticketInj.lastError().text()+"\n");
            statusbar->showMessage(ticketInj.lastError().text());
            return false;
        }
        int newId = GetId(t);
        if (newId > 0) t.injectID = newId;
    }
    case TICKET::Exist:
    {
        QString articleHeader = QString("INSERT INTO otrs.article(ticket_id,article_type_id,article_sender_type_id,a_content_type,a_body,incoming_time,content_path,valid_id,create_time,create_by,change_time,change_by) ");
        QString articleValues = QString("VALUES (<{ticket_id}>,9,1,\"text/plain; charset=utf-8\",<{a_body}>,<{incoming_time}>,<{content_path}>,1,<{create_time}>,4,<{change_time}>,4);");
        articleValues.replace("<{ticket_id}>",QString::number(t.injectID));
        bool msg_exist = false;
        QString body="\"";
        foreach (TICKET::Message m , t.messages)
        {
            if ((m.time>periodStart) && (m.time<periodEnd))
            {
                qDebug() << m.time.toString("hh:mm:ss dd.MM.yyyy") << ">" << periodStart.toString("hh:mm:ss dd.MM.yyyy") << "&& " << m.time.toString("hh:mm:ss dd.MM.yyyy") << "<" << periodEnd.toString("hh:mm:ss dd.MM.yyyy");
                msg_exist=true;
                body+=m.time.toString("hh:mm:ss dd.MM.yyyy\n");
                body+=(QString::fromUtf8(m.text.toAscii()));
            }
        }
        body+="\"";
        if (!msg_exist)
        {
            log->textCursor().insertText(QString::fromUtf8("Тикет №%1 не добавлен.\nНет изменений в обозначенный период\n").arg(t.ticket_number));
            return false;
        }
        articleValues.replace("<{a_body}>",body);
        articleValues.replace("<{incoming_time}>",QString::number(t.messages.at(0).time.toTime_t()));
        articleValues.replace("<{content_path}>",t.messages.at(0).time.toString("\"yyyy/mm/dd\""));
        articleValues.replace("<{create_time}>",t.messages.at(t.messages.size()-1).time.toString("\"yyyy-MM-dd hh:mm:ss\""));
        articleValues.replace("<{change_time}>",t.messages.at(t.messages.size()-1).time.toString("\"yyyy-MM-dd hh:mm:ss\""));
        QSqlQuery articleInj(articleHeader+articleValues,db);
        if (articleInj.lastError().type() != QSqlError::NoError)
        {
            qDebug() << articleHeader+articleValues << articleInj.lastError().text();
            log->textCursor().insertText(articleInj.lastError().text()+"\n");
            statusbar->showMessage(articleInj.lastError().text());
            return false;
        }else return true;
    }
        break;
    default:
        break;
    }
    return false;
}
