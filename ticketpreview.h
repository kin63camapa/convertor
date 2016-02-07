#ifndef TICKETPREVIEW_H
#define TICKETPREVIEW_H

#include <QWidget>
#include "ticket.h"

namespace Ui {
class TicketPreview;
}

class TicketPreview : public QWidget
{
    Q_OBJECT
public:
    explicit TicketPreview(QWidget *parent = 0);
    ~TicketPreview();
    void showTicket(TICKET t, int size);
    void showTicket(TICKET t);
signals:
    void nextBtnClicked();
    void prewBtnClicked();
    void save(TICKET);
private slots:
    void on_saveBtm_clicked();

private:
    Ui::TicketPreview *ui;  
};

#endif // TICKETPREVIEW_H
