#ifndef RESERVATIONPAGE_H
#define RESERVATIONPAGE_H

#include <QWidget>
#include <QDate>
#include "datastruct.h" // 包含预订数据结构

namespace Ui {
class ReservationPage;
}

class ReservationPage : public QWidget
{
    Q_OBJECT

public:
    explicit ReservationPage(QWidget *parent = nullptr);
    ~ReservationPage();

private slots:
    // 日期选择变化时刷新列表
    void refreshReservationList(const QDate &date);

    // 功能按钮点击事件
    void on_btn_Add_clicked();      // 新增预订
    void on_btn_Edit_clicked();     // 修改预订
    void on_btn_Delete_clicked();   // 删除预订

    // 状态下拉框变化时的处理（可选，用于实时更新状态）
    void onStatusChanged(int index);

private:
    Ui::ReservationPage *ui;

    // 辅助函数：初始化表格（设置列、样式等）
    void initTable();

    // 辅助函数：从表格当前行获取预订数据
    Reservation getReservationFromRow(int row);
};

#endif // RESERVATIONPAGE_H
