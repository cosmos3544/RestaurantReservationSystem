#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>
#include <QVector>
#include <QList>
#include "datastruct.h"

class DataCenter : public QObject
{
    Q_OBJECT

public:
    // 单例模式：全局唯一实例
    static DataCenter* instance();

    // 预订数据接口
    void addReservation(const Reservation& res);          // 新增预订
    void updateReservation(int index, const Reservation& res); // 修改预订
    void deleteReservation(const QDate& date, int index); // 删除预订
    QVector<Reservation> getReservationsByDate(const QDate& date); // 按日期获取预订

    // 餐桌数据接口
    void initTables(int tableCount, int capacityPerTable); // 初始化餐桌（桌数+每桌容纳人数）
    QVector<Table>& getAllTables();                        // 获取所有餐桌信息
    void addTableTimeRange(int tableId, const QDateTime& start, const QDateTime& end); // 新增餐桌时间区间
    void removeTableTimeRange(int tableId, const QDateTime& start, const QDateTime& end); // 删除餐桌时间区间

    // 日志数据接口
    void addLog(const Log& log);                          // 新增日志
    QVector<Log> getLogsByDate(const QDate& date);        // 按日期获取日志

    // 餐桌数据接口（增强）
    // 在原有 add/remove 基础上增加更稳健的替换和精确删除接口
    bool replaceTableTimeRangeCovering(int tableId, const QDateTime& newStart, const QDateTime& newEnd);
    void removeTableTimeRangeByExact(int tableId, const QDateTime& start, const QDateTime& end);


signals:
    // 数据变化信号：通知各模块刷新
    void reservationChanged(const QDate& date); // 某日期的预订变化
    void tableChanged();                        // 餐桌状态变化
    void logAdded();                            // 日志新增

private:
    // 单例模式：私有构造+静态实例
    explicit DataCenter(QObject *parent = nullptr);
    ~DataCenter() = default;
    DataCenter(const DataCenter&) = delete; // 禁止拷贝
    DataCenter& operator=(const DataCenter&) = delete; // 禁止赋值

    static DataCenter* m_instance; // 全局实例

    // 核心数据存储
    QVector<Reservation> m_allReservations; // 所有预订（不分日期）
    QVector<Table> m_tables;                // 所有餐桌
    QVector<Log> m_allLogs;                 // 所有日志
};

#endif // DATACENTER_H
