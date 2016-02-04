#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QSqlDatabase>
#include "parser.h"
#include "ticketpreview.h"

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
public slots:
    void openMail();
    void connectBase();
    void doAllZBS();
    void pbarinc(int i);
private:
    QSqlDatabase db;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QProgressBar *pbar;
    QLayout *l;
    QMenu *menu;
    QAction *openBase;
    QAction *openMailFolder;
    QPushButton *btn;
    TicketPreview *tab;
    QStringList *unknowEmails;
    Parser *parser;
};

#endif // MAINWINDOW_H
