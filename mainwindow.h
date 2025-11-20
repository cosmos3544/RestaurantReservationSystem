#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ReservationPage.h"
#include "TablePage.h"
#include "LogPage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void switchToTablePage(); // 之前的切换方法
    TablePage* getTablePage(); // 添加：获取TablePage实例的方法（供外部调用）

private slots:
    void on_btn_ReservationPage_clicked();
    void on_btn_TablePage_clicked();
    void on_btn_LogPage_clicked();

private:
    Ui::MainWindow *ui;
    ReservationPage *m_reservationPage;
    TablePage *m_tablePage; // 成员变量（已存在）
    LogPage *m_logPage;
};

#endif // MAINWINDOW_H
