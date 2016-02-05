#include "mainwindow.h"
#include "sqlconnectdialog.h"
#include "ticket.h"


MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    parser = new Parser(this);
    unknowEmails = new QStringList();
    tab = new TicketPreview(this);
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
        parser->initialization(unknowEmails,file,&db);
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
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Подключение к базе не выполнено!")).exec();
    }else
    {
        statusbar->showMessage("Connected:"+db.hostName()+" by name "+db.connectionName());
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
    msgBox.setText(QString::fromUtf8("Программа собирается сделать все збс, но может получится ровно наоборот!\nСобрано %1 тикетов\nИз них с неизвестными e-mail %2\nПодключен к базе %3\n").arg(QString::number(list.size())).arg(QString::number(unknowEmails->size())).arg(db.hostName()+":"+QString::number(db.port())));
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
           break;
       case QMessageBox::Discard:
           // manual
        tab->show();
        tab->showTicket(0);
           break;
       case QMessageBox::Cancel:
       default:
           return;
           break;
    }
}

void MainWindow::pbarinc(int i)
{
    pbar->setValue(pbar->maximum()-i);
}

int MainWindow::GetId(int i)
{
   //QSqlQuery q(QString("select id from otrs.ticket where tn like \"%1\" order by id;").arg(i),*db);
   //QSqlQuery q(QString("select ticket_state_id from otrs.ticket where tn like\"%1\" order by id;").arg(i),*db); //есть ощущение, что при значении 4 - заявка закрыта.
}
