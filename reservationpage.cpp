#include "ReservationPage.h"
#include "ui_ReservationPage.h"
#include "ReservationDialog.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QComboBox>
#include <datacenter.h>

ReservationPage::ReservationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReservationPage)
{
    ui->setupUi(this);
    setWindowTitle("预约管理");

    // 初始化表格
    initTable();

    // 初始化日期选择（默认当前日期）
    ui->date_Choose->setDate(QDate::currentDate());

    // 连接信号槽
    connect(DataCenter::instance(), &DataCenter::reservationChanged,
            this, &ReservationPage::refreshReservationList);

    connect(ui->date_Choose, &QDateEdit::dateChanged,
            this, &ReservationPage::refreshReservationList);
}

ReservationPage::~ReservationPage()
{
    delete ui;
}

// 初始化表格样式和列
void ReservationPage::initTable()
{
    // 设置列数和标题
    ui->table_ReservationInfo->setColumnCount(8);
    QStringList headers = {"姓名", "人数", "开始时间", "结束时间",
                           "桌号", "电话", "备注", "状态"};
    ui->table_ReservationInfo->setHorizontalHeaderLabels(headers);

    // 表格属性设置
    ui->table_ReservationInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 列自适应
    ui->table_ReservationInfo->setEditTriggers(QAbstractItemView::NoEditTriggers); // 单元格不可直接编辑（状态列通过ComboBox修改）
    ui->table_ReservationInfo->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
}

// 刷新预订列表
void ReservationPage::refreshReservationList(const QDate &date)
{
    ui->table_ReservationInfo->setRowCount(0);

    // 从DataCenter获取真实数据
    QVector<Reservation> reservations = DataCenter::instance()->getReservationsByDate(date);

    for (const auto &res : reservations) {
        int row = ui->table_ReservationInfo->rowCount();
        ui->table_ReservationInfo->insertRow(row);

        // 填充表格（和之前一致）
        ui->table_ReservationInfo->setItem(row, 0, new QTableWidgetItem(res.name));
        ui->table_ReservationInfo->setItem(row, 1, new QTableWidgetItem(QString::number(res.personCount)));
        ui->table_ReservationInfo->setItem(row, 2, new QTableWidgetItem(res.startTime.toString("HH:mm")));
        ui->table_ReservationInfo->setItem(row, 3, new QTableWidgetItem(res.endTime.toString("HH:mm")));
        ui->table_ReservationInfo->setItem(row, 4, new QTableWidgetItem(QString::number(res.tableId)));
        ui->table_ReservationInfo->setItem(row, 5, new QTableWidgetItem(res.phone));
        ui->table_ReservationInfo->setItem(row, 6, new QTableWidgetItem(res.notes));

        // 状态列ComboBox
        QComboBox *statusCombo = new QComboBox();
        statusCombo->addItems({"预约中", "已完成", "已取消"});
        statusCombo->setCurrentText(res.status);
        ui->table_ReservationInfo->setCellWidget(row, 7, statusCombo);

        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ReservationPage::onStatusChanged);
    }
}

// 从表格行提取预订数据
Reservation ReservationPage::getReservationFromRow(int row)
{
    Reservation res;
    if (row < 0 || row >= ui->table_ReservationInfo->rowCount()) {
        return res; // 无效行返回空数据
    }

    res.name = ui->table_ReservationInfo->item(row, 0)->text();
    res.personCount = ui->table_ReservationInfo->item(row, 1)->text().toInt();
    res.startTime = QDateTime(ui->date_Choose->date(),
                              QTime::fromString(ui->table_ReservationInfo->item(row, 2)->text(), "HH:mm"));
    res.endTime = QDateTime(ui->date_Choose->date(),
                            QTime::fromString(ui->table_ReservationInfo->item(row, 3)->text(), "HH:mm"));
    res.tableId = ui->table_ReservationInfo->item(row, 4)->text().toInt();
    res.phone = ui->table_ReservationInfo->item(row, 5)->text();
    res.notes = ui->table_ReservationInfo->item(row, 6)->text();

    QComboBox *combo = qobject_cast<QComboBox*>(ui->table_ReservationInfo->cellWidget(row, 7));
    if (combo) {
        res.status = combo->currentText();
    }

    return res;
}

// 2. 新增预订
void ReservationPage::on_btn_Add_clicked()
{
    ReservationDialog dialog(this);
    dialog.setReservationDate(ui->date_Choose->date());
    if (dialog.exec() == QDialog::Accepted) {
        Reservation res = dialog.getReservation();
        DataCenter::instance()->addReservation(res); // 存入DataCenter
        refreshReservationList(ui->date_Choose->date());
    }
}

// 3. 修改预订
void ReservationPage::on_btn_Edit_clicked()
{
    int currentRow = ui->table_ReservationInfo->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "提示", "请先选择一行预订记录");
        return;
    }

    // 获取当前日期的预订列表
    QVector<Reservation> reservations = DataCenter::instance()->getReservationsByDate(ui->date_Choose->date());
    if (currentRow >= reservations.size()) return;

    Reservation currentRes = reservations[currentRow];
    ReservationDialog dialog(this, currentRes);
    dialog.setReservationDate(ui->date_Choose->date());
    if (dialog.exec() == QDialog::Accepted) {
        Reservation newRes = dialog.getReservation();
        DataCenter::instance()->updateReservation(currentRow, newRes); // 更新DataCenter
        refreshReservationList(ui->date_Choose->date());
    }
}

// 4. 删除预订
void ReservationPage::on_btn_Delete_clicked()
{
    int currentRow = ui->table_ReservationInfo->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "提示", "请先选择一行预订记录");
        return;
    }

    if (QMessageBox::question(this, "确认", "是否删除该预订记录？") == QMessageBox::Yes) {
        // 通知DataCenter删除
        DataCenter::instance()->deleteReservation(ui->date_Choose->date(), currentRow);
        refreshReservationList(ui->date_Choose->date());
    }
}
// 状态下拉框变化时的处理（后续补充联动逻辑）
void ReservationPage::onStatusChanged(int index)
{
    Q_UNUSED(index);
    // 后续：当状态变为“已取消”时，通知TablePage释放桌位
}

