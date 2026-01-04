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
#include "data/DatabaseManager.h"

// 网络同步线程类（Qt 6多线程适配）
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
    QNetworkAccessManager* m_netManager; // 网络管理器（Qt 6）
    QThread* m_workerThread;             // 工作线程
    QTimer* m_syncTimer;                 // 同步定时器
    int m_syncInterval = 600;            // 默认同步间隔：10分钟（600秒）
    QString m_serverUrl = "http://127.0.0.1:8080/api/sync"; // 模拟服务器地址

    // 解析服务器JSON数据并同步到本地数据库
    void parseAndSyncData(const QByteArray& jsonData);
    // 构建网络请求（添加请求头）
    QNetworkRequest buildRequest();
};

#endif // NETWORKWORKER_H