#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// 全局内联日志函数（避免重复定义，支持模块区分）
inline void writeLog(const QString& level, const QString& msg, const QString& module = "COMMON") {
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/classboard.log";
    QDir().mkpath(QFileInfo(logPath).absolutePath());

    QFile logFile(logPath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
               << " [" << module << "] [" << level << "] " << msg << "\n";
        logFile.close();
    }
}

#endif // LOGHELPER_H
