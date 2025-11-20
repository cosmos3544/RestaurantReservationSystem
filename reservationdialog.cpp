#include "ReservationDialog.h"
#include "ui_ReservationDialog.h"
#include "MainWindow.h"
#include "TablePage.h"
#include <QMessageBox>

ReservationDialog::ReservationDialog(QWidget *parent, const Reservation &res)
    : QDialog(parent)
    , ui(new Ui::ReservationDialog)
    , m_originalRes(res)
{
    ui->setupUi(this);
    setWindowTitle(res.id == -1 ? "新增预订" : "修改预订");

    // 初始化时间选择
    ui->time_Start->setDisplayFormat("HH:mm");
    ui->time_End->setDisplayFormat("HH:mm");
    ui->time_Start->setMinimumTime(QTime(10, 0));
    ui->time_End->setMaximumTime(QTime(22, 0));

    // 填充修改数据
    if (res.id != -1) {
        ui->edit_Name->setText(res.name);
        ui->spin_PersonCount->setValue(res.personCount);
        ui->time_Start->setTime(res.startTime.time());
        ui->time_End->setTime(res.endTime.time());
        ui->spin_TableId->setValue(res.tableId);
        ui->edit_Phone->setText(res.phone);
        ui->edit_Notes->setText(res.notes);
        m_reservationDate = res.startTime.date(); // 从原始数据获取日期
    } else {
        // 新增时默认时间
        ui->time_Start->setTime(QTime::currentTime());
        ui->time_End->setTime(QTime::currentTime().addSecs(3600));
    }
}

ReservationDialog::~ReservationDialog()
{
    delete ui;
}

// 接收父窗口传入的日期
void ReservationDialog::setReservationDate(const QDate &date)
{
    m_reservationDate = date;
}

// 获取预订信息（修复QDateTime转换错误）
Reservation ReservationDialog::getReservation() const
{
    Reservation res = m_originalRes;
    res.name = ui->edit_Name->text();
    res.personCount = ui->spin_PersonCount->value();
    // 正确构造QDateTime（日期+时间）
    res.startTime = QDateTime(m_reservationDate, ui->time_Start->time());
    res.endTime = QDateTime(m_reservationDate, ui->time_End->time());
    res.tableId = ui->spin_TableId->value();
    res.phone = ui->edit_Phone->text();
    res.notes = ui->edit_Notes->toPlainText();
    return res;
}

void ReservationDialog::on_btn_Confirm_clicked()
{
    if (ui->edit_Name->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写预约者姓名");
        return;
    }
    if (ui->edit_Phone->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写联系电话");
        return;
    }
    if (ui->time_Start->time() >= ui->time_End->time()) {
        QMessageBox::warning(this, "时间错误", "开始时间必须早于结束时间");
        return;
    }
    accept();
}

void ReservationDialog::on_btn_Cancel_clicked()
{
    reject();
}

void ReservationDialog::on_btn_CheckTable_clicked()
{
    // 1. 日期直接用 setReservationDate() 给你的 m_reservationDate
    QDate selectedDate = m_reservationDate;

    QTime startTime = ui->time_Start->time();
    QTime endTime = ui->time_End->time();

    if (!startTime.isValid() || !endTime.isValid() || startTime >= endTime) {
        QMessageBox::warning(this, "输入错误", "请选择有效的时间段（开始时间早于结束时间）");
        return;
    }

    MainWindow *mainWindow = qobject_cast<MainWindow*>(this->parentWidget());
    if (!mainWindow) {
        mainWindow = qobject_cast<MainWindow*>(QApplication::activeWindow());
    }
    if (!mainWindow) {
        QMessageBox::warning(this, "错误", "无法获取主窗口");
        return;
    }

    TablePage *tablePage = mainWindow->getTablePage();
    if (!tablePage) {
        QMessageBox::warning(this, "错误", "餐桌管理页面未初始化");
        return;
    }

    // 4. 调用 TablePage
    tablePage->showTableStatus(selectedDate, startTime, endTime);

    mainWindow->switchToTablePage();
}
