#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "开始构造MainWindow"; // 调试输出
    ui->setupUi(this);
    setWindowTitle("餐厅预订管理系统");

    // 检查ui->stacked_Main是否存在（关键！）
    if (!ui->stacked_Main) {
        qDebug() << "错误：stacked_Main不存在！"; // 若输出此句，说明UI文件有误
    }

    // 初始化子页面
    m_reservationPage = new ReservationPage();
    m_tablePage = new TablePage();
    m_logPage = new LogPage();
    qDebug() << "子页面初始化完成";

    // 添加到栈窗口
    if (ui->stacked_Main) {
        ui->stacked_Main->addWidget(m_reservationPage);
        ui->stacked_Main->addWidget(m_tablePage);
        ui->stacked_Main->addWidget(m_logPage);
        ui->stacked_Main->setCurrentWidget(m_reservationPage);
        qDebug() << "子页面添加到stacked_Main完成";
    }

    qDebug() << "MainWindow构造完成";
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_reservationPage;
    delete m_tablePage;
    delete m_logPage;
}

// 切换到预订管理页
void MainWindow::on_btn_ReservationPage_clicked()
{
    ui->stacked_Main->setCurrentWidget(m_reservationPage);
}

// 切换到餐桌管理页（解决LNK2019错误的关键）
void MainWindow::on_btn_TablePage_clicked()
{
    ui->stacked_Main->setCurrentWidget(m_tablePage);
}

// 切换到日志页
void MainWindow::on_btn_LogPage_clicked()
{
    ui->stacked_Main->setCurrentWidget(m_logPage);
}

// 添加到 MainWindow.cpp 中
void MainWindow::switchToTablePage()
{
    ui->stacked_Main->setCurrentWidget(m_tablePage);
}

// 添加 getTablePage 实现
TablePage* MainWindow::getTablePage()
{
    return m_tablePage; // 返回 TablePage 实例
}
