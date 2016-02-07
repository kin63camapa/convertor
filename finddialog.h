#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();
    enum FindType
    {
        byId,
        byNum,
        byPos,
        byStr
    };
signals:
    void fnd(FindDialog::FindType,QString);
private slots:
    void on_fBtn_clicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
