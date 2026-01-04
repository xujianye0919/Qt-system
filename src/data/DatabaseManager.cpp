#include "DatabaseManager.h"

// 单例实例初始化
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

// 初始化数据库
bool DatabaseManager::init(const QString& dbPath)
{
    // 关闭已存在的连接（Qt 6需避免重复连接）
    if (QSqlDatabase::contains("classboard_conn")) {
        m_db = QSqlDatabase::database("classboard_conn");
        if (m_db.isOpen()) return true;
    }

    // 设置数据库路径（优先使用传入路径，否则默认AppData目录）
    if (dbPath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir appDir(appDataPath);
        if (!appDir.exists()) {
            if (!appDir.mkpath(".")) {
                emit operateFailed("创建AppData目录失败");
                return false;
            }
        }
        m_dbPath = appDir.filePath(m_dbName);
    } else {
        m_dbPath = dbPath;
    }

    // 加载SQLite驱动（Qt 6需确保sql模块已引入）
    m_db = QSqlDatabase::addDatabase("QSQLITE", "classboard_conn");
    m_db.setDatabaseName(m_dbPath);

    // 打开数据库
    if (!m_db.open()) {
        QString errMsg = QString("数据库打开失败：%1").arg(m_db.lastError().text());
        qCritical() << errMsg;
        emit operateFailed(errMsg);
        return false;
    }

    // 创建数据表
    createTables();
    qInfo() << "数据库初始化成功，路径：" << m_dbPath;
    emit operateSuccess("数据库初始化成功");
    return true;
}

// 创建数据表（检查表是否存在）
void DatabaseManager::createTables()
{
    QSqlQuery query(m_db);
    QFile sqlFile(":/sql/create_tables.sql"); // 后续将SQL文件加入资源文件

    // 读取SQL脚本并执行
    if (sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString sql = sqlFile.readAll();
        sqlFile.close();
        // Qt 6支持批量执行SQL（分隔符;）
        QStringList sqlList = sql.split(";", Qt::SkipEmptyParts);
        for (const QString& sqlStmt : sqlList) {
            if (!query.exec(sqlStmt.trimmed())) {
                qWarning() << "建表失败：" << query.lastError().text();
            }
        }
    } else {
        // 备用方案：直接执行建表SQL（避免资源文件问题）
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS class_info (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                room_number TEXT NOT NULL UNIQUE,
                class_name TEXT NOT NULL,
                grade TEXT NOT NULL,
                department TEXT NOT NULL
            )
        )");
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS course_schedule (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                class_id INTEGER NOT NULL,
                course_name TEXT NOT NULL,
                teacher TEXT NOT NULL,
                course_type TEXT NOT NULL,
                start_time TEXT NOT NULL,
                end_time TEXT NOT NULL,
                day_of_week INTEGER NOT NULL,
                start_date TEXT NOT NULL,
                end_date TEXT NOT NULL,
                classroom TEXT,
                FOREIGN KEY(class_id) REFERENCES class_info(id) ON DELETE CASCADE
            )
        )");
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS notices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                content TEXT NOT NULL,
                publish_time TEXT NOT NULL,
                expire_time TEXT,
                is_scrolling INTEGER DEFAULT 0,
                is_valid INTEGER DEFAULT 1
            )
        )");
        // 创建索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_course_class_id ON course_schedule(class_id)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_course_date ON course_schedule(start_date, end_date)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_notices_valid ON notices(is_valid, expire_time)");
    }
}

// -------------------------- 班级管理实现 --------------------------
bool DatabaseManager::addClass(const QString& roomNumber, const QString& className, const QString& grade, const QString& department)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO class_info (room_number, class_name, grade, department)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(roomNumber);
    query.addBindValue(className);
    query.addBindValue(grade);
    query.addBindValue(department);

    if (query.exec()) {
        emit operateSuccess("班级添加成功");
        return true;
    } else {
        QString errMsg = QString("班级添加失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

bool DatabaseManager::deleteClass(int classId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM class_info WHERE id = ?");
    query.addBindValue(classId);

    if (query.exec()) {
        emit operateSuccess("班级删除成功");
        return true;
    } else {
        QString errMsg = QString("班级删除失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

QList<QVariantMap> DatabaseManager::getAllClasses()
{
    QList<QVariantMap> classList;
    QSqlQuery query("SELECT id, room_number, class_name, grade, department FROM class_info ORDER BY id", m_db);

    while (query.next()) {
        QVariantMap classMap;
        classMap["id"] = query.value("id").toInt();
        classMap["room_number"] = query.value("room_number").toString();
        classMap["class_name"] = query.value("class_name").toString();
        classMap["grade"] = query.value("grade").toString();
        classMap["department"] = query.value("department").toString();
        classList.append(classMap);
    }
    return classList;
}

QList<QVariantMap> DatabaseManager::searchClasses(const QString& keyword)
{
    QList<QVariantMap> classList;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT id, room_number, class_name, grade, department FROM class_info
        WHERE class_name LIKE ? OR room_number LIKE ? OR department LIKE ?
        ORDER BY id
    )");
    QString likeKeyword = QString("%%1%").arg(keyword);
    query.addBindValue(likeKeyword);
    query.addBindValue(likeKeyword);
    query.addBindValue(likeKeyword);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap classMap;
            classMap["id"] = query.value("id").toInt();
            classMap["room_number"] = query.value("room_number").toString();
            classMap["class_name"] = query.value("class_name").toString();
            classMap["grade"] = query.value("grade").toString();
            classMap["department"] = query.value("department").toString();
            classList.append(classMap);
        }
    } else {
        qWarning() << "班级搜索失败：" << query.lastError().text();
    }
    return classList;
}

// -------------------------- 课表管理实现 --------------------------
bool DatabaseManager::addCourse(int classId, const QString& courseName, const QString& teacher, const QString& courseType,
                               const QString& startTime, const QString& endTime, int dayOfWeek,
                               const QString& startDate, const QString& endDate, const QString& classroom)
{
    // 验证日期格式
    QString formattedStart = formatDate(startDate);
    QString formattedEnd = formatDate(endDate);
    if (formattedStart.isEmpty() || formattedEnd.isEmpty()) {
        emit operateFailed("日期格式错误（请使用YYYY-MM-DD）");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO course_schedule (class_id, course_name, teacher, course_type,
                                    start_time, end_time, day_of_week, start_date, end_date, classroom)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(classId);
    query.addBindValue(courseName);
    query.addBindValue(teacher);
    query.addBindValue(courseType);
    query.addBindValue(startTime);
    query.addBindValue(endTime);
    query.addBindValue(dayOfWeek);
    query.addBindValue(formattedStart);
    query.addBindValue(formattedEnd);
    query.addBindValue(classroom);

    if (query.exec()) {
        emit operateSuccess("课程添加成功");
        return true;
    } else {
        QString errMsg = QString("课程添加失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

bool DatabaseManager::deleteCourse(int courseId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM course_schedule WHERE id = ?");
    query.addBindValue(courseId);

    if (query.exec()) {
        emit operateSuccess("课程删除成功");
        return true;
    } else {
        QString errMsg = QString("课程删除失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

QList<QVariantMap> DatabaseManager::getCoursesByClassId(int classId)
{
    QList<QVariantMap> courseList;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT id, course_name, teacher, course_type, start_time, end_time, day_of_week,
               start_date, end_date, classroom
        FROM course_schedule
        WHERE class_id = ? AND start_date <= ? AND end_date >= ?
        ORDER BY day_of_week, start_time
    )");
    query.addBindValue(classId);
    query.addBindValue(today);
    query.addBindValue(today);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap courseMap;
            courseMap["id"] = query.value("id").toInt();
            courseMap["course_name"] = query.value("course_name").toString();
            courseMap["teacher"] = query.value("teacher").toString();
            courseMap["course_type"] = query.value("course_type").toString();
            courseMap["start_time"] = query.value("start_time").toString();
            courseMap["end_time"] = query.value("end_time").toString();
            courseMap["day_of_week"] = query.value("day_of_week").toInt();
            courseMap["start_date"] = query.value("start_date").toString();
            courseMap["end_date"] = query.value("end_date").toString();
            courseMap["classroom"] = query.value("classroom").toString();
            courseList.append(courseMap);
        }
    } else {
        qWarning() << "课程查询失败：" << query.lastError().text();
    }
    return courseList;
}

QVariantMap DatabaseManager::getCurrentCourse(int classId)
{
    QVariantMap currentCourse;
    QDate today = QDate::currentDate();
    QTime now = QTime::currentTime();
    QString todayStr = today.toString("yyyy-MM-dd");
    int dayOfWeek = today.dayOfWeek();

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT id, course_name, teacher, course_type, start_time, end_time, classroom
        FROM course_schedule
        WHERE class_id = ? AND day_of_week = ? AND start_date <= ? AND end_date >= ?
              AND start_time <= ? AND end_time >= ?
    )");
    query.addBindValue(classId);
    query.addBindValue(dayOfWeek);
    query.addBindValue(todayStr);
    query.addBindValue(todayStr);
    query.addBindValue(now.toString("HH:mm"));
    query.addBindValue(now.toString("HH:mm"));

    if (query.exec() && query.next()) {
        currentCourse["id"] = query.value("id").toInt();
        currentCourse["course_name"] = query.value("course_name").toString();
        currentCourse["teacher"] = query.value("teacher").toString();
        currentCourse["course_type"] = query.value("course_type").toString();
        currentCourse["start_time"] = query.value("start_time").toString();
        currentCourse["end_time"] = query.value("end_time").toString();
        currentCourse["classroom"] = query.value("classroom").toString();
    }
    return currentCourse;
}

QVariantMap DatabaseManager::getNextCourse(int classId)
{
    QVariantMap nextCourse;
    QDate today = QDate::currentDate();
    QTime now = QTime::currentTime();
    QString todayStr = today.toString("yyyy-MM-dd");
    int dayOfWeek = today.dayOfWeek();

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT id, course_name, teacher, course_type, start_time, end_time, classroom
        FROM course_schedule
        WHERE class_id = ? AND day_of_week = ? AND start_date <= ? AND end_date >= ?
              AND start_time > ?
        ORDER BY start_time ASC LIMIT 1
    )");
    query.addBindValue(classId);
    query.addBindValue(dayOfWeek);
    query.addBindValue(todayStr);
    query.addBindValue(todayStr);
    query.addBindValue(now.toString("HH:mm"));

    if (query.exec() && query.next()) {
        nextCourse["id"] = query.value("id").toInt();
        nextCourse["course_name"] = query.value("course_name").toString();
        nextCourse["teacher"] = query.value("teacher").toString();
        nextCourse["course_type"] = query.value("course_type").toString();
        nextCourse["start_time"] = query.value("start_time").toString();
        nextCourse["end_time"] = query.value("end_time").toString();
        nextCourse["classroom"] = query.value("classroom").toString();
    }
    return nextCourse;
}

// -------------------------- 通知管理实现 --------------------------
bool DatabaseManager::addNotice(const QString& title, const QString& content, const QString& publishTime,
                               const QString& expireTime, bool isScrolling)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO notices (title, content, publish_time, expire_time, is_scrolling, is_valid)
        VALUES (?, ?, ?, ?, ?, 1)
    )");
    query.addBindValue(title);
    query.addBindValue(content);
    query.addBindValue(publishTime);
    query.addBindValue(expireTime);
    query.addBindValue(isScrolling ? 1 : 0);

    if (query.exec()) {
        emit operateSuccess("通知添加成功");
        return true;
    } else {
        QString errMsg = QString("通知添加失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

bool DatabaseManager::deleteNotice(int noticeId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notices WHERE id = ?");
    query.addBindValue(noticeId);

    if (query.exec()) {
        emit operateSuccess("通知删除成功");
        return true;
    } else {
        QString errMsg = QString("通知删除失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

bool DatabaseManager::updateNoticeStatus(int noticeId, bool isScrolling, bool isValid)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE notices SET is_scrolling = ?, is_valid = ? WHERE id = ?
    )");
    query.addBindValue(isScrolling ? 1 : 0);
    query.addBindValue(isValid ? 1 : 0);
    query.addBindValue(noticeId);

    if (query.exec()) {
        emit operateSuccess("通知状态更新成功");
        return true;
    } else {
        QString errMsg = QString("通知状态更新失败：%1").arg(query.lastError().text());
        emit operateFailed(errMsg);
        return false;
    }
}

QList<QVariantMap> DatabaseManager::getValidNotices(bool isScrolling)
{
    QList<QVariantMap> noticeList;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    if (isScrolling) {
        query.prepare(R"(
            SELECT id, title, content, publish_time, expire_time
            FROM notices
            WHERE is_valid = 1 AND is_scrolling = 1
                  AND (expire_time IS NULL OR expire_time >= ?)
            ORDER BY publish_time DESC
        )");
        query.addBindValue(today);
    } else {
        query.prepare(R"(
            SELECT id, title, content, publish_time, expire_time, is_scrolling
            FROM notices
            WHERE is_valid = 1 AND (expire_time IS NULL OR expire_time >= ?)
            ORDER BY publish_time DESC
        )");
        query.addBindValue(today);
    }

    if (query.exec()) {
        while (query.next()) {
            QVariantMap noticeMap;
            noticeMap["id"] = query.value("id").toInt();
            noticeMap["title"] = query.value("title").toString();
            noticeMap["content"] = query.value("content").toString();
            noticeMap["publish_time"] = query.value("publish_time").toString();
            noticeMap["expire_time"] = query.value("expire_time").toString();
            if (!isScrolling) {
                noticeMap["is_scrolling"] = query.value("is_scrolling").toInt() == 1;
            }
            noticeList.append(noticeMap);
        }
    } else {
        qWarning() << "通知查询失败：" << query.lastError().text();
    }
    return noticeList;
}

// -------------------------- 辅助函数 --------------------------
bool DatabaseManager::isDateInRange(const QString& checkDate, const QString& startDate, const QString& endDate)
{
    QDate check = QDate::fromString(checkDate, "yyyy-MM-dd");
    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");
    return check.isValid() && start.isValid() && end.isValid() && check >= start && check <= end;
}

QString DatabaseManager::formatDate(const QString& dateStr)
{
    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (date.isValid()) {
        return date.toString("yyyy-MM-dd");
    }
    // 兼容其他格式（如MM/dd/yyyy）
    date = QDate::fromString(dateStr, "MM/dd/yyyy");
    if (date.isValid()) {
        return date.toString("yyyy-MM-dd");
    }
    return "";
}