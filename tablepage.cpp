#include "TablePage.h"
#include "ui_TablePage.h"
#include "DataCenter.h"
#include <QDateTime>
#include <QColor>

TablePage::TablePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TablePage)
    , m_timer(new QTimer(this))
    , m_targetDate(QDate::currentDate())
{
    ui->setupUi(this);
    setWindowTitle("餐桌管理");

    initTable();

    ui->combo_TimeSlot->addItems({
        "10:00-12:00",
        "12:00-14:00",
        "16:00-18:00",
        "18:00-20:00",
        "20:00-22:00"
    });

    // 默认启用实时模式
    ui->rBtn_RealTime->setChecked(true);
    on_rBtn_RealTime_toggled(true);

    // 定时器每分钟更新一次实时显示
    m_timer->setInterval(60000);
    connect(m_timer, &QTimer::timeout, this, &TablePage::refreshRealTimeData);
    m_timer->start();
    refreshRealTimeData();

    // 连接 DataCenter 的 tableChanged 信号（当 DataCenter 表格数据变化时刷新）
    connect(DataCenter::instance(), &DataCenter::tableChanged,
            this, &TablePage::refreshTableStatus);

    // **新增连接：当某个日期的预订发生变化时（新增/删除/修改），如果当前查看的日期与之相同，刷新显示**
    connect(DataCenter::instance(), &DataCenter::reservationChanged,
            this, &TablePage::onReservationChanged);
}

TablePage::~TablePage()
{
    m_timer->stop();
    delete ui;
}

// 安全获取表格项（避免空指针）
QTableWidgetItem* TablePage::getSafeTableItem(int row, int column)
{
    if (row < 0 || row >= ui->table_TableInfo->rowCount() ||
        column < 0 || column >= ui->table_TableInfo->columnCount()) {
        return nullptr;
    }

    QTableWidgetItem *item = ui->table_TableInfo->item(row, column);
    if (!item) {
        item = new QTableWidgetItem("");
        ui->table_TableInfo->setItem(row, column, item);
    }
    return item;
}

// 初始化表格（从 DataCenter 加载餐桌数据）
void TablePage::initTable()
{
    ui->table_TableInfo->clear();

    ui->table_TableInfo->setColumnCount(5);
    QStringList headers = {"桌号", "容纳人数", "状态", "开始时间", "结束时间"};
    ui->table_TableInfo->setHorizontalHeaderLabels(headers);
    ui->table_TableInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_TableInfo->setSelectionBehavior(QTableWidget::SelectRows);

    QVector<Table> tables = DataCenter::instance()->getAllTables();
    if (tables.isEmpty()) {
        qDebug() << "DataCenter 无餐桌数据，自动初始化 10 张桌";
        DataCenter::instance()->initTables(10, 4);
        tables = DataCenter::instance()->getAllTables();
    }

    ui->table_TableInfo->setRowCount(tables.size());

    for (int i = 0; i < tables.size(); ++i) {
        const Table &table = tables[i];

        QTableWidgetItem *item = nullptr;

        item = getSafeTableItem(i, 0);
        item->setText(QString::number(table.id));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        item = getSafeTableItem(i, 1);
        item->setText(QString::number(table.capacity));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        item = getSafeTableItem(i, 2);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        item = getSafeTableItem(i, 3);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        item = getSafeTableItem(i, 4);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

// 刷新表格状态（核心逻辑）
// 说明：函数内所有对表格的程序化写入都被包裹在 m_programmaticUpdate 与 blockSignals 中，
// 以避免触发 on_table_TableInfo_cellChanged 的用户处理逻辑，从而防止递归/无限循环。
void TablePage::refreshTableStatus()
{
    // 如果已经处于程序化更新中，直接返回（防止重入）
    if (m_programmaticUpdate) return;

    m_programmaticUpdate = true;
    ui->table_TableInfo->blockSignals(true);

    QVector<Table> tables = DataCenter::instance()->getAllTables();
    QDateTime now = QDateTime::currentDateTime();

    // 如果行数不一致，重建
    if (ui->table_TableInfo->rowCount() != tables.size()) {
        initTable();
    }

    for (int i = 0; i < tables.size(); ++i) {
        const Table &table = tables[i];

        QTableWidgetItem *statusItem = getSafeTableItem(i, 2);
        QTableWidgetItem *startItem = getSafeTableItem(i, 3);
        QTableWidgetItem *endItem = getSafeTableItem(i, 4);

        if (!statusItem || !startItem || !endItem)
            continue;

        // 默认：清空显示
        startItem->setText("");
        endItem->setText("");
        statusItem->setText("");
        statusItem->setBackground(QColor());

        // ----- 实时模式 -----
        if (ui->rBtn_RealTime->isChecked()) {
            bool occupied = false;
            QDateTime occStart, occEnd;

            for (const auto &range : table.timeRanges) {
                if (now >= range.first && now <= range.second) {
                    occupied = true;
                    occStart = range.first;
                    occEnd = range.second;
                    break;
                }
            }

            if (occupied) {
                startItem->setText(occStart.toString("HH:mm"));
                endItem->setText(occEnd.toString("HH:mm"));

                int remaining = now.secsTo(occEnd) / 60;
                if (remaining <= 15) {
                    statusItem->setText("即将结束");
                    statusItem->setBackground(QColor(255, 215, 0));
                } else {
                    statusItem->setText("占用中");
                    statusItem->setBackground(QColor(255, 100, 100));
                }

                // 实时模式下被占用的桌允许编辑时间（临时调整）
                startItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                endItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            } else {
                statusItem->setText("空闲");
                statusItem->setBackground(QColor(100, 255, 100));

                // 非占用不允许编辑
                startItem->setFlags(Qt::ItemIsEnabled);
                endItem->setFlags(Qt::ItemIsEnabled);
            }
        }
        // ----- 时间段模式 -----
        else {
            QDateTime tStart(m_targetDate, m_targetStart);
            QDateTime tEnd(m_targetDate, m_targetEnd);

            bool occ = isTableOccupied(table.id, tStart, tEnd);

            if (occ) {
                statusItem->setText("已预订");
                statusItem->setBackground(QColor(255, 100, 100));

                // 找到与目标区间有交集的已有区间并显示（优先显示已有预约的原始时间）
                for (const auto &range : table.timeRanges) {
                    if (range.first < tEnd && range.second > tStart) {
                        startItem->setText(range.first.toString("HH:mm"));
                        endItem->setText(range.second.toString("HH:mm"));
                        break;
                    }
                }
            } else {
                statusItem->setText("可预订");
                statusItem->setBackground(QColor(100, 255, 100));
                // 非占用：保持开始/结束为空（不把 m_targetStart 写到每一行）
                startItem->setText("");
                endItem->setText("");
            }

            // 时间段模式不允许用户直接编辑表格时间
            startItem->setFlags(Qt::ItemIsEnabled);
            endItem->setFlags(Qt::ItemIsEnabled);
        }
    }

    ui->table_TableInfo->blockSignals(false);
    m_programmaticUpdate = false;
}

// 判断餐桌在指定时间区间是否被占用（有交集则视为占用）
bool TablePage::isTableOccupied(int tableId, const QDateTime &start, const QDateTime &end)
{
    QVector<Table> tables = DataCenter::instance()->getAllTables();
    for (const auto &table : tables) {
        if (table.id != tableId) continue;

        for (const auto &range : table.timeRanges) {
            if (range.first < end && range.second > start) {
                return true;
            }
        }
        break;
    }
    return false;
}

// 实时模式切换
void TablePage::on_rBtn_RealTime_toggled(bool checked)
{
    if (checked) {
        ui->label_SystemTime->setEnabled(true);
        ui->combo_TimeSlot->setEnabled(false);

        for (int i = 0; i < ui->table_TableInfo->rowCount(); ++i) {
            QTableWidgetItem *startItem = getSafeTableItem(i, 3);
            QTableWidgetItem *endItem = getSafeTableItem(i, 4);
            if (startItem) startItem->setFlags(startItem->flags() | Qt::ItemIsEditable);
            if (endItem) endItem->setFlags(endItem->flags() | Qt::ItemIsEditable);
        }

        refreshTableStatus();
    }
}

// 时间段模式切换
void TablePage::on_rBtn_TimeRange_toggled(bool checked)
{
    if (checked) {
        ui->label_SystemTime->setEnabled(false);
        ui->combo_TimeSlot->setEnabled(true);

        for (int i = 0; i < ui->table_TableInfo->rowCount(); ++i) {
            QTableWidgetItem *startItem = getSafeTableItem(i, 3);
            QTableWidgetItem *endItem = getSafeTableItem(i, 4);
            if (startItem) startItem->setFlags(startItem->flags() & ~Qt::ItemIsEditable);
            if (endItem) endItem->setFlags(endItem->flags() & ~Qt::ItemIsEditable);
        }

        // 使用 combo 的当前选项初始化时间段
        on_combo_TimeSlot_currentIndexChanged( ui->combo_TimeSlot->currentIndex() );
    }
}

// 时间段选择变化
void TablePage::on_combo_TimeSlot_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString slot = ui->combo_TimeSlot->currentText();
    QStringList times = slot.split("-");
    if (times.size() == 2) {
        m_targetStart = QTime::fromString(times[0], "HH:mm");
        m_targetEnd = QTime::fromString(times[1], "HH:mm");
        refreshTableStatus();
    }
}

// 实时模式定时刷新
void TablePage::refreshRealTimeData()
{
    ui->label_SystemTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    refreshTableStatus();
}

// 表格单元格编辑完成（实时模式下修改时间）
void TablePage::on_table_TableInfo_cellChanged(int row, int column)
{
    // 1. 如果是程序化更新产生的，忽略
    if (m_programmaticUpdate)
        return;

    // 2. 仅在实时模式下，且修改的是开始/结束列才处理
    if (!ui->rBtn_RealTime->isChecked() || (column != 3 && column != 4))
        return;

    QTableWidgetItem *tableIdItem = getSafeTableItem(row, 0);
    QTableWidgetItem *startItem = getSafeTableItem(row, 3);
    QTableWidgetItem *endItem = getSafeTableItem(row, 4);

    if (!tableIdItem || !startItem || !endItem)
        return;

    int tableId = tableIdItem->text().toInt();

    QString s = startItem->text().trimmed();
    QString e = endItem->text().trimmed();

    // ❗❗ 3. 允许空白输入（用户还没打完），不做任何处理
    if (s.isEmpty() || e.isEmpty())
        return;

    QTime st = QTime::fromString(s, "HH:mm");
    QTime et = QTime::fromString(e, "HH:mm");

    // 4. 如果格式错误，不刷新，不改回，只是提示
    if (!st.isValid() || !et.isValid() || st >= et) {
        QMessageBox::warning(this, "时间错误", "格式必须为 HH:mm 且开始早于结束");
        return;
    }

    // 5. 如果格式正确，写入 DataCenter
    QDateTime dst(QDate::currentDate(), st);
    QDateTime det(QDate::currentDate(), et);

    bool replaced = DataCenter::instance()->replaceTableTimeRangeCovering(tableId, dst, det);
    if (!replaced)
        DataCenter::instance()->addTableTimeRange(tableId, dst, det);

    // 6. 程序化刷新一次表格显示
    m_programmaticUpdate = true;
    refreshTableStatus();
    m_programmaticUpdate = false;
}


// ReservationDialog / DataCenter 新增/删除预订时触发（带日期）
// 如果当前 TablePage 正查看的日期与变动日期相同，则刷新显示
void TablePage::onReservationChanged(const QDate &date)
{
    if (date == m_targetDate) {
        refreshTableStatus();
    }
}

// 显示指定日期和时间段的餐桌状态（供预订弹窗调用）
void TablePage::showTableStatus(const QDate &date, const QTime &start, const QTime &end)
{
    if (!start.isValid() || !end.isValid() || start >= end) {
        qDebug() << "无效时间段";
        return;
    }

    // 防止切换时重复触发多次刷新
    m_programmaticUpdate = true;
    ui->rBtn_TimeRange->blockSignals(true);
    ui->rBtn_TimeRange->setChecked(true);
    ui->rBtn_TimeRange->blockSignals(false);
    m_programmaticUpdate = false;

    m_targetDate = date;
    m_targetStart = start;
    m_targetEnd = end;

    refreshTableStatus();
}
