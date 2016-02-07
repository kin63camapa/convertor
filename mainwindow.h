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
    void parsetFinished();
    void pbarinc(int i);
    void newTicket(TICKET t);
    void editTicket(TICKET t);
    void injectTicket(TICKET t);
    void nextBtnClicked();
    void prewBtnClicked();
private:
    QLayout *l;
    QMenu *menu;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QProgressBar *pbar;
    QAction *openBase;
    QAction *openMailFolder;
    QPushButton *btn;
    QPlainTextEdit *log;
    TicketPreview *tab;
    int lastId;
    int index;
    QStringList unknowEmails;
    Parser *parser;
    QSqlDatabase db;
    QList<TICKET> list;
    QDateTime periodStart;
    QDateTime periodEnd;

    int GetId(TICKET t);
    TICKET getUserinfo(TICKET *t);
    TICKET::Status getState(TICKET *t);
    bool inject(TICKET t);

};

#endif // MAINWINDOW_H
