#include "NetworkWorker.h"

NetworkWorker::NetworkWorker(QObject *parent) : QObject(parent)
{
    // 创建工作线程（Qt 6线程安全）
    m_workerThread = new QThread(this);
    this->moveToThread(m_workerThread);
    m_workerThread->start();

    // 初始化网络管理器 + 设置全局超时（Qt 6推荐方式）
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setTransferTimeout(10000); // 设置10秒超时（替代原TimeoutAttribute）
    connect(m_netManager, &QNetworkAccessManager::finished, this, &NetworkWorker::onReplyFinished);

    // 初始化同步定时器
    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(m_syncInterval * 1000);
    connect(m_syncTimer, &QTimer::timeout, this, &NetworkWorker::onSyncTimerTimeout);
    connect(this, &NetworkWorker::startSyncTimer, m_syncTimer, QOverload<>::of(&QTimer::start));

    // 从设置管理器获取服务器地址
    m_serverUrl = SettingsManager::instance().getServerUrl();
    writeLog("INFO", "网络模块初始化成功，服务器地址：" + m_serverUrl, "NETWORK");
}

NetworkWorker::~NetworkWorker()
{
    // 线程安全退出
    m_syncTimer->stop();
    m_workerThread->quit();
    if (!m_workerThread->wait(3000)) {
        m_workerThread->terminate();
        writeLog("WARNING", "网络线程强制退出", "NETWORK");
    }
    writeLog("INFO", "网络模块已销毁", "NETWORK");
}

// 手动触发同步
void NetworkWorker::triggerSync()
{
    QMetaObject::invokeMethod(this, "onSyncTimerTimeout", Qt::QueuedConnection);
    writeLog("INFO", "手动触发数据同步", "NETWORK");
}

// 定时同步任务
void NetworkWorker::onSyncTimerTimeout()
{
    // 更新服务器地址（可能已修改）
    m_serverUrl = SettingsManager::instance().getServerUrl();

    // 发送GET请求
    QNetworkRequest request = buildRequest();
    writeLog("INFO", "开始同步数据，服务器地址：" + m_serverUrl, "NETWORK");
    m_netManager->get(request);
}

// 处理网络响应
void NetworkWorker::onReplyFinished(QNetworkReply* reply)
{
    static int retryCount = 0; // 重试计数器

    if (reply->error() != QNetworkReply::NoError) {
        QString errMsg = QString("网络请求失败：%1").arg(reply->errorString());
        writeLog("ERROR", errMsg, "NETWORK");

        // 断网重试（最多2次）
        if (retryCount < 2) {
            retryCount++;
            writeLog("INFO", QString("断网重试（第%1次），3秒后重试").arg(retryCount), "NETWORK");
            QTimer::singleShot(3000, this, &NetworkWorker::onSyncTimerTimeout);
        } else {
            retryCount = 0;
            emit syncFailed(errMsg);
        }

        reply->deleteLater();
        return;
    }

    // 重置重试计数器
    retryCount = 0;

    // 读取响应数据
    QByteArray jsonData = reply->readAll();
    reply->deleteLater();
    writeLog("INFO", "收到服务器响应，数据长度：" + QString::number(jsonData.size()), "NETWORK");

    // 解析并同步到数据库
    parseAndSyncData(jsonData);

    emit syncSuccess("数据同步成功！");
    writeLog("INFO", "数据同步完成", "NETWORK");
}

// 构建网络请求
QNetworkRequest NetworkWorker::buildRequest()
{
    QUrl url(m_serverUrl);
    QNetworkRequest request(url); // 正确创建QNetworkRequest对象

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "ClassBoardSystem/1.0 (Qt 6.9.2)");
    request.setRawHeader("Accept", "application/json");

    // 【删除这行】Qt 6已移除TimeoutAttribute，超时已通过m_netManager->setTransferTimeout设置
    // request.setAttribute(QNetworkRequest::TimeoutAttribute, 10000);

    // 设置缓存策略（Qt 6兼容）
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    return request;
}

// 解析JSON并同步到本地数据库
void NetworkWorker::parseAndSyncData(const QByteArray& jsonData)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString errMsg = QString("JSON解析失败：%1").arg(parseError.errorString());
        writeLog("ERROR", errMsg, "NETWORK");
        emit syncFailed(errMsg);
        return;
    }

    if (!doc.isObject()) {
        writeLog("ERROR", "服务器返回非JSON对象", "NETWORK");
        emit syncFailed("服务器返回数据格式错误（非JSON对象）");
        return;
    }

    QJsonObject root = doc.object();
    if (root["code"].toInt() != 200) {
        QString errMsg = QString("服务器返回错误：%1").arg(root["msg"].toString());
        writeLog("ERROR", errMsg, "NETWORK");
        emit syncFailed(errMsg);
        return;
    }

    // 解析班级数据
    QJsonArray classArray = root["classes"].toArray();
    writeLog("INFO", "解析班级数据，数量：" + QString::number(classArray.size()), "NETWORK");

    for (const QJsonValue& val : classArray) {
        QJsonObject obj = val.toObject();
        QString roomNumber = obj["room_number"].toString();
        QString className = obj["class_name"].toString();
        QString grade = obj["grade"].toString();
        QString department = obj["department"].toString();

        // 同步逻辑：先删后加
        DatabaseManager::instance().deleteClass(obj["id"].toInt());
        DatabaseManager::instance().addClass(roomNumber, className, grade, department);
    }

    // 解析课程数据
    QJsonArray courseArray = root["courses"].toArray();
    writeLog("INFO", "解析课程数据，数量：" + QString::number(courseArray.size()), "NETWORK");

    for (const QJsonValue& val : courseArray) {
        QJsonObject obj = val.toObject();
        int classId = obj["class_id"].toInt();
        QString courseName = obj["course_name"].toString();
        QString teacher = obj["teacher"].toString();
        QString courseType = obj["course_type"].toString();
        QString startTime = obj["start_time"].toString();
        QString endTime = obj["end_time"].toString();
        int dayOfWeek = obj["day_of_week"].toInt();
        QString startDate = obj["start_date"].toString();
        QString endDate = obj["end_date"].toString();
        QString classroom = obj["classroom"].toString();

        DatabaseManager::instance().deleteCourse(obj["id"].toInt());
        DatabaseManager::instance().addCourse(classId, courseName, teacher, courseType,
                                             startTime, endTime, dayOfWeek,
                                             startDate, endDate, classroom);
    }

    // 解析通知数据
    QJsonArray noticeArray = root["notices"].toArray();
    writeLog("INFO", "解析通知数据，数量：" + QString::number(noticeArray.size()), "NETWORK");

    for (const QJsonValue& val : noticeArray) {
        QJsonObject obj = val.toObject();
        QString title = obj["title"].toString();
        QString content = obj["content"].toString();
        QString publishTime = obj["publish_time"].toString();
        QString expireTime = obj["expire_time"].toString();
        bool isScrolling = obj["is_scrolling"].toBool();

        DatabaseManager::instance().deleteNotice(obj["id"].toInt());
        DatabaseManager::instance().addNotice(title, content, publishTime, expireTime, isScrolling);
    }
}
