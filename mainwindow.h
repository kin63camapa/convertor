#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QSqlDatabase>

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

signals:

public slots:
    void openMail();
    void connectBase();
    void tst();
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
};

#endif // MAINWINDOW_H
