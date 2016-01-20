#include "mainwindow.h"
#include "sqlconnectdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{

    tab = new QTableWidget(this);
    statusbar = new QStatusBar(this);

    menubar = new QMenuBar(this);
    menu = new QMenu(menubar);
    openBase = new QAction(QString::fromUtf8("Открыть Базу"),this);
    menu->setTitle("File");
    menu->addAction(openBase);
    menubar->addMenu(menu);
    connect(openBase,SIGNAL(triggered()),this,SLOT(connectBase()));
    btn = new QPushButton(this);
    connect(btn,SIGNAL(clicked()),this,SLOT(tst()));
    l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(menubar);
    l->addWidget(tab);
    l->addWidget(btn);
    l->addWidget(statusbar);
    connectBase();
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

    QSqlQuery q("select title,first_name from otrs.users where login like \"natuko\" order by id;",db);
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
