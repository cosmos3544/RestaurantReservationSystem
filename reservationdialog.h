#ifndef RESERVATIONDIALOG_H
#define RESERVATIONDIALOG_H

#include <QDialog>
#include "datastruct.h"

namespace Ui {
class ReservationDialog;
}

class ReservationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReservationDialog(QWidget *parent = nullptr, const Reservation &res = Reservation());
    ~ReservationDialog();

    Reservation getReservation() const;

    // 设置预订日期（从父窗口传入，避免直接访问ui）
    void setReservationDate(const QDate &date);

private slots:
    void on_btn_Confirm_clicked();
    void on_btn_Cancel_clicked();
    void on_btn_CheckTable_clicked();

private:
    Ui::ReservationDialog *ui;
    Reservation m_originalRes;
    QDate m_reservationDate; // 存储预订日期（替代直接访问父窗口ui）
};

#endif // RESERVATIONDIALOG_H
