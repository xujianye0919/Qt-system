#include "DatabaseManager.h"
#include <QRegularExpression>

// 单例实例初始化
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

// 初始化数据库（带重试逻辑，无清空旧数据）
bool DatabaseManager::init(const QString& dbPath)
{
    // 关闭已存在的连接
    if (QSqlDatabase::contains("classboard_conn")) {
        m_db = QSqlDatabase::database("classboard_conn");
        if (m_db.isOpen()) {
            writeLog("INFO", "数据库已连接", "DATABASE");
            return true;
        }
    }

    // 设置数据库路径
    if (dbPath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir appDir(appDataPath);
        if (!appDir.exists() && !appDir.mkpath(".")) {
            writeLog("ERROR", "创建AppData目录失败", "DATABASE");
            emit operateFailed("创建AppData目录失败");
            return false;
        }
        m_dbPath = appDir.filePath(m_dbName);
    } else {
        m_dbPath = dbPath;
    }

    // 重试连接（最多3次）
    int retryCount = 0;
    while (retryCount < 3) {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "classboard_conn");
        m_db.setDatabaseName(m_dbPath);

        if (m_db.open()) {
            writeLog("INFO", "数据库连接成功", "DATABASE");
            break;
        }

        retryCount++;
        writeLog("ERROR", QString("数据库连接失败（重试%1次）：%2").arg(retryCount).arg(m_db.lastError().text()), "DATABASE");
        QThread::msleep(500); // 休眠500ms重试
    }

    if (!m_db.isOpen()) {
        QString errMsg = QString("数据库打开失败（重试3次）：%1").arg(m_db.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
        emit operateFailed(errMsg);
        return false;
    }

    // 创建数据表
    createTables();

    writeLog("INFO", "数据库初始化成功", "DATABASE");
    emit operateSuccess("数据库初始化成功");
    return true;
}

// 辅助函数：清理SQL语句中的单行注释和空白（仅保留这一个定义）
QString DatabaseManager::cleanSqlStatement(const QString& stmt)
{
    // 移除单行注释（-- 到行尾）
    QString cleanStmt = stmt;
    QRegularExpression commentRegex("--.*$", QRegularExpression::MultilineOption);
    cleanStmt = cleanStmt.replace(commentRegex, "");
    // 移除多余空白（换行、制表符、多个空格）
    cleanStmt = cleanStmt.replace(QRegularExpression("\\s+"), " ");
    cleanStmt = cleanStmt.trimmed();
    return cleanStmt;
}

// 创建数据表（移除所有内置SQL，仅保留资源文件读取逻辑）
void DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // -------------------------- 读取建表脚本 --------------------------
    QFile sqlFile(":/sql/create_tables.sql");

    if (sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 处理UTF-8 BOM
        QByteArray content = sqlFile.readAll();
        if (content.startsWith("\xef\xbb\xbf")) {
            content = content.mid(3);
        }
        QString sql = QString::fromUtf8(content);
        sqlFile.close();

        // 分割SQL语句（按分号分割，保留有效语句）
        QStringList sqlList = sql.split(";", Qt::SkipEmptyParts);

        // 先执行建表语句，再执行索引语句（保证顺序）
        QStringList createTableStmts; // 建表语句
        QStringList createIndexStmts; // 索引语句

        for (int i = 0; i < sqlList.size(); i++) {
            QString rawStmt = sqlList[i];
            QString cleanStmt = cleanSqlStatement(rawStmt); // 清理注释和空白

            // 跳过清理后为空的语句
            if (cleanStmt.isEmpty()) {
                continue;
            }

            // 分类：建表/索引语句
            if (cleanStmt.startsWith("CREATE TABLE", Qt::CaseInsensitive) ||
                cleanStmt.startsWith("DROP TABLE", Qt::CaseInsensitive)) {
                createTableStmts.append(cleanStmt);
            } else if (cleanStmt.startsWith("CREATE INDEX", Qt::CaseInsensitive)) {
                createIndexStmts.append(cleanStmt);
            }

            // 执行当前语句
            if (!query.exec(cleanStmt)) {
                writeLog("WARNING", QString("建表语句执行失败：%1").arg(query.lastError().text()), "DATABASE");
            }
        }
    } else {
        // 移除内置SQL后，仅打印错误日志，不执行任何建表操作
        writeLog("ERROR", "未找到建表脚本：:/sql/create_tables.sql，无法创建数据表", "DATABASE");
    }

    // -------------------------- 导入测试数据（仅当无班级数据时） --------------------------
    // 先检查class_info是否已有数据
    query.exec("SELECT COUNT(*) FROM class_info");
    int classCount = 0;
    if (query.next()) {
        classCount = query.value(0).toInt();
    }

    if (classCount > 0) {
        return;
    }

    QFile testSqlFile(":/sql/test_data.sql");
    if (testSqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 处理UTF-8 BOM
        QByteArray testContent = testSqlFile.readAll();
        if (testContent.startsWith("\xef\xbb\xbf")) {
            testContent = testContent.mid(3);
        }
        QString testSql = QString::fromUtf8(testContent);
        testSqlFile.close();

        // 分割SQL语句（按分号分割）
        QStringList testSqlList = testSql.split(";", Qt::SkipEmptyParts);

        for (int i = 0; i < testSqlList.size(); i++) {
            QString rawStmt = testSqlList[i];
            QString cleanStmt = cleanSqlStatement(rawStmt); // 清理注释和空白

            // 跳过清理后为空的语句
            if (cleanStmt.isEmpty()) {
                continue;
            }

            // 执行清理后的有效SQL
            if (!query.exec(cleanStmt)) {
                writeLog("ERROR", QString("测试数据语句执行失败：%1").arg(query.lastError().text()), "DATABASE");
            }
        }
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
        writeLog("INFO", "添加班级成功：" + className, "DATABASE");
        emit operateSuccess("班级添加成功");
        return true;
    } else {
        QString errMsg = QString("班级添加失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
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
        writeLog("INFO", "删除班级成功，ID：" + QString::number(classId), "DATABASE");
        emit operateSuccess("班级删除成功");
        return true;
    } else {
        QString errMsg = QString("班级删除失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
        emit operateFailed(errMsg);
        return false;
    }
}

QList<QVariantMap> DatabaseManager::getAllClasses()
{
    QList<QVariantMap> classList;
    QSqlQuery query(m_db); // 先创建查询对象，绑定数据库
    // 准备并执行查询（关键：必须调用exec()）
    QString sql = "SELECT id, room_number, class_name, grade, department FROM class_info ORDER BY id";
    if (!query.exec(sql)) { // 执行查询并检查是否成功
        QString errMsg = "查询所有班级失败：" + query.lastError().text();
        writeLog("ERROR", errMsg, "DATABASE");
        emit operateFailed(errMsg);
        return classList; // 执行失败返回空列表
    }

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
        QString errMsg = "班级搜索失败：" + query.lastError().text();
        writeLog("ERROR", errMsg, "DATABASE");
        emit operateFailed(errMsg);
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
        writeLog("ERROR", "日期格式错误：" + startDate + " / " + endDate, "DATABASE");
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
        writeLog("INFO", "添加课程成功：" + courseName, "DATABASE");
        emit operateSuccess("课程添加成功");
        return true;
    } else {
        QString errMsg = QString("课程添加失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
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
        writeLog("INFO", "删除课程成功，ID：" + QString::number(courseId), "DATABASE");
        emit operateSuccess("课程删除成功");
        return true;
    } else {
        QString errMsg = QString("课程删除失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
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
        QString errMsg = "课程查询失败：" + query.lastError().text();
        writeLog("ERROR", errMsg, "DATABASE");
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

// -------------------------- 通知管理实现（修复参数不匹配） --------------------------
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
        writeLog("INFO", "添加通知成功：" + title, "DATABASE");
        emit operateSuccess("通知添加成功");
        return true;
    } else {
        QString errMsg = QString("通知添加失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
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
        writeLog("INFO", "删除通知成功，ID：" + QString::number(noticeId), "DATABASE");
        emit operateSuccess("通知删除成功");
        return true;
    } else {
        QString errMsg = QString("通知删除失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
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
        writeLog("INFO", QString("更新通知状态成功，ID：%1").arg(noticeId), "DATABASE");
        emit operateSuccess("通知状态更新成功");
        return true;
    } else {
        QString errMsg = QString("通知状态更新失败：%1").arg(query.lastError().text());
        writeLog("ERROR", errMsg, "DATABASE");
        emit operateFailed(errMsg);
        return false;
    }
}

QList<QVariantMap> DatabaseManager::getValidNotices(bool isScrolling)
{
    QList<QVariantMap> noticeList;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QSqlQuery query(m_db);
    QString sql;
    // 修复参数数量不匹配：统一SQL模板，仅调整筛选条件
    if (isScrolling) {
        sql = R"(
            SELECT id, title, content, publish_time, expire_time
            FROM notices
            WHERE is_valid = 1 AND is_scrolling = 1
                  AND (expire_time IS NULL OR expire_time >= ?)
            ORDER BY publish_time DESC
        )";
    } else {
        sql = R"(
            SELECT id, title, content, publish_time, expire_time, is_scrolling
            FROM notices
            WHERE is_valid = 1 AND (expire_time IS NULL OR expire_time >= ?)
            ORDER BY publish_time DESC
        )";
    }

    // 预处理+绑定参数（仅1个参数，避免数量不匹配）
    query.prepare(sql);
    query.addBindValue(today); // 仅绑定1个参数，适配两种场景

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
        QString errMsg = "通知查询失败：" + query.lastError().text();
        writeLog("ERROR", errMsg, "DATABASE");
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
    // 兼容MM/dd/yyyy格式
    date = QDate::fromString(dateStr, "MM/dd/yyyy");
    if (date.isValid()) {
        return date.toString("yyyy-MM-dd");
    }
    return "";
}
