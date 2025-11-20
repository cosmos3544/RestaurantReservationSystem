#include "DataCenter.h"
#include <QDateTime>
#include <algorithm>

// 初始化静态实例
DataCenter* DataCenter::m_instance = nullptr;

DataCenter* DataCenter::instance()
{
    if (!m_instance) {
        m_instance = new DataCenter();
    }
    return m_instance;
}

DataCenter::DataCenter(QObject *parent)
    : QObject(parent)
{
    initTables(10, 4);
}

// ------------------ 预订接口（你原有的函数可以保留） ------------------

void DataCenter::addReservation(const Reservation &res)
{
    Reservation newRes = res;
    newRes.id = m_allReservations.size() + 1;
    m_allReservations.append(newRes);

    // 使用增强的 add（会合并/去重）
    addTableTimeRange(newRes.tableId, newRes.startTime, newRes.endTime);

    emit reservationChanged(newRes.startTime.date());
}

void DataCenter::updateReservation(int index, const Reservation &res)
{
    if (index < 0 || index >= m_allReservations.size()) return;

    Reservation oldRes = m_allReservations[index];
    // 删除旧区间（按精确匹配）
    removeTableTimeRangeByExact(oldRes.tableId, oldRes.startTime, oldRes.endTime);

    m_allReservations[index] = res;
    addTableTimeRange(res.tableId, res.startTime, res.endTime);

    emit reservationChanged(res.startTime.date());
}

void DataCenter::deleteReservation(const QDate &date, int index)
{
    QVector<Reservation> targetRes = getReservationsByDate(date);
    if (index < 0 || index >= targetRes.size()) return;

    Reservation deletedRes = targetRes[index];
    m_allReservations.removeOne(deletedRes);

    removeTableTimeRangeByExact(deletedRes.tableId, deletedRes.startTime, deletedRes.endTime);

    Log log;
    log.operateTime = QDateTime::currentDateTime();
    log.deletedRes = deletedRes;
    addLog(log);

    emit reservationChanged(date);
    emit tableChanged();
}

QVector<Reservation> DataCenter::getReservationsByDate(const QDate &date)
{
    QVector<Reservation> result;
    for (const auto& res : m_allReservations) {
        if (res.startTime.date() == date) {
            result.append(res);
        }
    }
    return result;
}

// ------------------ 餐桌数据接口（增强实现） ------------------

void DataCenter::initTables(int tableCount, int capacityPerTable)
{
    m_tables.clear();
    for (int i = 1; i <= tableCount; ++i) {
        Table table;
        table.id = i;
        table.capacity = capacityPerTable;
        m_tables.append(table);
    }
    emit tableChanged();
}

QVector<Table>& DataCenter::getAllTables()
{
    return m_tables;
}

// Helper: 合并并规范化一个 table 的 timeRanges（把有交集或相邻的区间合并成最小区间集合）
static void normalizeTimeRanges(QList<QPair<QDateTime, QDateTime>>& ranges) {
    if (ranges.size() <= 1) return;
    // 转换为 vector 并按 start 排序
    std::sort(ranges.begin(), ranges.end(), [](const QPair<QDateTime, QDateTime>& a, const QPair<QDateTime, QDateTime>& b){
        return a.first < b.first;
    });
    QList<QPair<QDateTime, QDateTime>> merged;
    QPair<QDateTime, QDateTime> cur = ranges[0];
    for (int i = 1; i < ranges.size(); ++i) {
        auto &r = ranges[i];
        // 如果当前区间与下一区间有交集或相邻（r.first <= cur.second），合并
        if (r.first <= cur.second) {
            // 取更晚的结束时间
            if (r.second > cur.second) cur.second = r.second;
        } else {
            merged.append(cur);
            cur = r;
        }
    }
    merged.append(cur);
    ranges = merged;
}

void DataCenter::addTableTimeRange(int tableId, const QDateTime &start, const QDateTime &end)
{
    if (!start.isValid() || !end.isValid() || start >= end) return;

    for (auto& table : m_tables) {
        if (table.id == tableId) {
            // 直接 append 后规范化（合并重叠/相邻区间，避免重复）
            table.timeRanges.append(qMakePair(start, end));
            normalizeTimeRanges(table.timeRanges);
            emit tableChanged();
            break;
        }
    }
}

// 旧的 remove 仍然保留，但我们提供更健壮的 removeByExact
void DataCenter::removeTableTimeRange(int tableId, const QDateTime &start, const QDateTime &end)
{
    // 为兼容旧调用，尝试按精确匹配删除（和以前一致）
    removeTableTimeRangeByExact(tableId, start, end);
}

void DataCenter::removeTableTimeRangeByExact(int tableId, const QDateTime &start, const QDateTime &end)
{
    for (auto& table : m_tables) {
        if (table.id == tableId) {
            for (int i = 0; i < table.timeRanges.size(); ++i) {
                QPair<QDateTime, QDateTime> range = table.timeRanges[i];
                if (range.first == start && range.second == end) {
                    table.timeRanges.removeAt(i);
                    emit tableChanged();
                    break;
                }
            }
            // 规范化以防万一
            normalizeTimeRanges(table.timeRanges);
            break;
        }
    }
}

// 新增：替换某个与 new 区间有交集的已有区间（用于用户编辑时修改已有预约）
// 找到与 newStart/newEnd 有交集的第一个区间并替换，返回 true；否则返回 false（调用者可选择直接 add）
bool DataCenter::replaceTableTimeRangeCovering(int tableId, const QDateTime &newStart, const QDateTime &newEnd)
{
    if (!newStart.isValid() || !newEnd.isValid() || newStart >= newEnd) return false;

    for (auto& table : m_tables) {
        if (table.id == tableId) {
            int found = -1;
            for (int i = 0; i < table.timeRanges.size(); ++i) {
                auto &r = table.timeRanges[i];
                // 判断是否有交集： r.first < newEnd && r.second > newStart
                if (r.first < newEnd && r.second > newStart) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                // 用新区间替换旧区间，然后规范化（以免产生重叠）
                table.timeRanges[found].first = newStart;
                table.timeRanges[found].second = newEnd;
                normalizeTimeRanges(table.timeRanges);
                emit tableChanged();
                return true;
            }
            break;
        }
    }
    return false;
}

// ------------------ 日志接口 ------------------

void DataCenter::addLog(const Log &log)
{
    m_allLogs.append(log);
    emit logAdded();
}

QVector<Log> DataCenter::getLogsByDate(const QDate &date)
{
    QVector<Log> result;
    for (const auto& log : m_allLogs) {
        if (log.operateTime.date() == date) {
            result.append(log);
        }
    }
    return result;
}
