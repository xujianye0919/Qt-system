#include "NetworkWorker.h"

NetworkWorker::NetworkWorker(QObject *parent) : QObject(parent)
{
    // 创建工作线程（Qt 6线程安全）
    m_workerThread = new QThread(this);
    this->moveToThread(m_workerThread);
    m_workerThread->start();

    // 初始化网络管理器（Qt 6需在主线程创建？NO，线程内创建）
    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &NetworkWorker::onReplyFinished);

    // 初始化同步定时器
    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(m_syncInterval * 1000);
    connect(m_syncTimer, &QTimer::timeout, this, &NetworkWorker::onSyncTimerTimeout);
    connect(this, &NetworkWorker::startSyncTimer, m_syncTimer, QOverload<>::of(&QTimer::start));
}

NetworkWorker::~NetworkWorker()
{
    // 线程安全退出（Qt 6推荐方式）
    m_syncTimer->stop();
    m_workerThread->quit();
    m_workerThread->wait(3000); // 等待3秒退出
}

// 手动触发同步
void NetworkWorker::triggerSync()
{
    QMetaObject::invokeMethod(this, "onSyncTimerTimeout", Qt::QueuedConnection);
}

// 定时同步任务
void NetworkWorker::onSyncTimerTimeout()
{
    // 发送GET请求到服务器（Qt 6网络请求）
    QNetworkRequest request = buildRequest();
    m_netManager->get(request);
}

// 处理网络响应
void NetworkWorker::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QString errMsg = QString("网络请求失败：%1").arg(reply->errorString());
        emit syncFailed(errMsg);
        reply->deleteLater();
        return;
    }

    // 读取响应数据
    QByteArray jsonData = reply->readAll();
    reply->deleteLater();

    // 解析并同步到数据库
    parseAndSyncData(jsonData);
    emit syncSuccess("数据同步成功！");
}

// 构建网络请求
QNetworkRequest NetworkWorker::buildRequest()
{
    QNetworkRequest request(QUrl(m_serverUrl));
    // 设置请求头（JSON格式）
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "ClassBoardSystem/1.0 (Qt 6.9.2)");
    // 设置超时（10秒）
    request.setAttribute(QNetworkRequest::TimeoutAttribute, 10000);
    return request;
}

// 解析JSON并同步到本地数据库
void NetworkWorker::parseAndSyncData(const QByteArray& jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        emit syncFailed("服务器返回数据格式错误（非JSON对象）");
        return;
    }

    QJsonObject root = doc.object();
    if (root["code"].toInt() != 200) {
        emit syncFailed(QString("服务器返回错误：%1").arg(root["msg"].toString()));
        return;
    }

    // 解析班级数据
    QJsonArray classArray = root["classes"].toArray();
    for (const QJsonValue& val : classArray) {
        QJsonObject obj = val.toObject();
        QString roomNumber = obj["room_number"].toString();
        QString className = obj["class_name"].toString();
        QString grade = obj["grade"].toString();
        QString department = obj["department"].toString();

        // 先删除旧数据，再添加新数据（简化同步逻辑）
        DatabaseManager::instance().deleteClass(obj["id"].toInt());
        DatabaseManager::instance().addClass(roomNumber, className, grade, department);
    }

    // 解析课程数据
    QJsonArray courseArray = root["courses"].toArray();
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