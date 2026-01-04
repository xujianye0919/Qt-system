#ifndef TIMEHELPER_H
#define TIMEHELPER_H

#include <QObject>
#include <QTime>
#include <QString>
#include <QDateTime>

// 时间工具类（Qt 6适配）
class TimeHelper : public QObject
{
    Q_OBJECT
public:
    explicit TimeHelper(QObject *parent = nullptr) : QObject(parent) {}

    // 计算倒计时（目标时间：HH:mm格式，返回XX:XX:XX）
    static QString getCountdown(const QString& targetTimeStr);

    // 判断当前时间是否在[startTime, endTime]范围内
    static bool isTimeInRange(const QString& startTimeStr, const QString& endTimeStr);

    // 格式化时间差为XX小时XX分XX秒
    static QString formatTimeDiff(int seconds);

private:
    // 解析时间字符串为QTime（HH:mm）
    static QTime parseTime(const QString& timeStr);
};

#endif // TIMEHELPER_H