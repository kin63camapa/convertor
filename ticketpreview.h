#ifndef TICKETPREVIEW_H
#define TICKETPREVIEW_H

#include <QWidget>

namespace Ui {
class TicketPreview;
}

class TicketPreview : public QWidget
{
    Q_OBJECT

public:
    explicit TicketPreview(QWidget *parent = 0);
    ~TicketPreview();
    void showTicket(int i);

private slots:
    void on_nextBtn_clicked();

    void on_prewBtn_clicked();

private:
    Ui::TicketPreview *ui;
    int index;
};

#endif // TICKETPREVIEW_H
