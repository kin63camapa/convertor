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
    /* QStringList a;
    a.push_back("*.*");
    QString str, QDir(QFileDialog::getExistingDirectory(this)),a;
    {
        QFile file(str);
        file.open(QIODevice::ReadOnly); //Открываем файл.
        QList<TICKET> list;



    }
    */
    QFile *file = new QFile(QFileDialog::getOpenFileName(this));
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Warning,QString::fromUtf8("Апшипка"),QString::fromUtf8("Не удалось открыть файл: %1").arg(file->fileName())).exec();
    }
    else
    {
        while(!file->atEnd()){
            QString str = file->readLine();
            if(str.contains(QRegExp("From [0-9]@xxx")))
                qDebug() << str;


            //if (str.contains(""))
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
