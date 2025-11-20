#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <QDateTime>
#include <QString>

// 预订信息结构体
struct Reservation {
    int id = -1;               // 预订ID（-1表示未初始化，唯一标识）
    QString name;              // 预约者姓名
    int personCount = 0;       // 人数
    QDateTime startTime;       // 开始时间（含日期）
    QDateTime endTime;         // 结束时间（含日期）
    int tableId = 0;           // 桌号
    QString phone;             // 电话
    QString notes;             // 备注
    QString status = "预约中"; // 状态：预约中/已完成/已取消

    // 重载 == 运算符：通过id判断两个预订是否相等
    bool operator==(const Reservation& other) const {
        return this->id == other.id;
    }
};

// 餐桌信息结构体
struct Table {
    int id = 0;                // 桌号
    int capacity = 0;          // 容纳人数
    // 存储该餐桌的所有时间区间（包括预约和临时占用）
    QList<QPair<QDateTime, QDateTime>> timeRanges;
};

// 日志信息结构体
struct Log {
    QDateTime operateTime;     // 操作时间（删除预订的时间）
    Reservation deletedRes;    // 被删除的预订信息
};

#endif // DATASTRUCT_H
