#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QSqlDatabase>

struct Userinfo
{
    bool isOk;
    QString customer_user_id;
    QString customer_id;
};

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

signals:

public slots:
    void openMail();
    void connectBase();

private:
    QSqlDatabase db;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QLayout *l;
    QMenu *menu;
    QAction *openBase;
    QAction *openMailFolder;
    QPushButton *btn;
    QTableWidget *tab;
    Userinfo getFromEmail(QString email);
    QStringList *unknowEmails;
};

#endif // MAINWINDOW_H
