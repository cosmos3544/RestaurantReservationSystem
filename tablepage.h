#ifndef TABLEPAGE_H
#define TABLEPAGE_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include "datastruct.h"
#include <QTableWidgetItem>

namespace Ui {
class TablePage;
}

class TablePage : public QWidget
{
    Q_OBJECT

public:
    explicit TablePage(QWidget *parent = nullptr);
    ~TablePage();

    // 显示指定日期和时间段的餐桌状态（供ReservationDialog调用）
    void showTableStatus(const QDate &date, const QTime &start, const QTime &end);

private slots:
    void on_rBtn_RealTime_toggled(bool checked);
    void on_rBtn_TimeRange_toggled(bool checked);

    void on_combo_TimeSlot_currentIndexChanged(int index);

    void refreshRealTimeData();

    void on_table_TableInfo_cellChanged(int row, int column);

    // 新增：当 DataCenter 的预订发生变化（某日期）时调用
    void onReservationChanged(const QDate &date);

private:
    Ui::TablePage *ui;
    QTimer *m_timer;

    QDate m_targetDate;
    QTime m_targetStart;
    QTime m_targetEnd;

    // 程序化更新时置为 true，避免在 setText / setFlags 时触发 cellChanged 处理逻辑
    bool m_programmaticUpdate = false;

    void initTable();
    void refreshTableStatus();
    bool isTableOccupied(int tableId, const QDateTime &start, const QDateTime &end);

    QTableWidgetItem* getSafeTableItem(int row, int column);
};

#endif // TABLEPAGE_H
