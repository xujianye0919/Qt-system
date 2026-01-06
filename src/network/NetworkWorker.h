#ifndef NETWORKWORKER_H
#define NETWORKWORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "data/DatabaseManager.h"
#include "settings/SettingsManager.h"
#include "utility/LogHelper.h" // 包含公共日志头文件

// 网络同步线程类（Qt 6.9.2多线程适配）
class NetworkWorker : public QObject
{
    Q_OBJECT
public:
    explicit NetworkWorker(QObject *parent = nullptr);
    ~NetworkWorker();

    // 设置同步间隔（秒）
    void setSyncInterval(int secs) { m_syncInterval = secs; }

    // 手动触发同步
    void triggerSync();

signals:
    // 同步结果通知
    void syncSuccess(const QString& msg);
    void syncFailed(const QString& msg);
    // 内部信号：启动定时同步
    void startSyncTimer();

private slots:
    // 定时同步任务
    void onSyncTimerTimeout();
    // 处理网络响应
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_netManager; // 网络管理器
    QThread* m_workerThread;             // 工作线程
    QTimer* m_syncTimer;                 // 同步定时器
    int m_syncInterval = 600;            // 默认10分钟
    QString m_serverUrl;                 // 服务器地址

    // 辅助函数
    void parseAndSyncData(const QByteArray& jsonData);
    QNetworkRequest buildRequest();
    int getClassroomIdByName(const QString& classroomName);
};

#endif // NETWORKWORKER_H
