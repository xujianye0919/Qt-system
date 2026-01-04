#include "TimeHelper.h"

// 计算倒计时（目标时间为当天的HH:mm）
QString TimeHelper::getCountdown(const QString& targetTimeStr)
{
    QTime now = QTime::currentTime();
    QTime target = parseTime(targetTimeStr);

    if (!target.isValid()) {
        return "00:00:00";
    }

    // 计算秒数差（目标时间 - 当前时间）
    int diffSec = now.secsTo(target);
    if (diffSec <= 0) {
        return "00:00:00"; // 已结束
    }

    return formatTimeDiff(diffSec);
}

// 判断当前时间是否在时间段内
bool TimeHelper::isTimeInRange(const QString& startTimeStr, const QString& endTimeStr)
{
    QTime now = QTime::currentTime();
    QTime start = parseTime(startTimeStr);
    QTime end = parseTime(endTimeStr);

    if (!start.isValid() || !end.isValid()) {
        return false;
    }

    return now >= start && now <= end;
}

// 格式化秒数为XX:XX:XX
QString TimeHelper::formatTimeDiff(int seconds)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    return QString("%1:%2:%3")
           .arg(hours, 2, 10, QChar('0'))
           .arg(minutes, 2, 10, QChar('0'))
           .arg(secs, 2, 10, QChar('0'));
}

// 解析HH:mm格式时间
QTime TimeHelper::parseTime(const QString& timeStr)
{
    return QTime::fromString(timeStr, "HH:mm");
}