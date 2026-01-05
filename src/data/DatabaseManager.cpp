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
            writeLog("INFO", "数据库已连接，路径：" + m_dbPath, "DATABASE");
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
            writeLog("INFO", QString("数据库连接成功（重试%1次），路径：%2").arg(retryCount).arg(m_dbPath), "DATABASE");
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

    writeLog("INFO", "数据库初始化成功，路径：" + m_dbPath, "DATABASE");
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

// 创建数据表（修复建表语句解析+执行顺序）
void DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // -------------------------- 读取建表脚本 --------------------------
    QFile sqlFile(":/sql/create_tables.sql");
    writeLog("INFO", "尝试读取建表脚本：:/sql/create_tables.sql", "DATABASE");

    if (sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 处理UTF-8 BOM
        QByteArray content = sqlFile.readAll();
        if (content.startsWith("\xef\xbb\xbf")) {
            content = content.mid(3);
            writeLog("INFO", "检测到UTF-8 BOM，已移除", "DATABASE");
        }
        QString sql = QString::fromUtf8(content);
        sqlFile.close();

        writeLog("INFO", "建表脚本内容（前200字符）：" + sql.left(200) + "...", "DATABASE");

        // 分割SQL语句（按分号分割，保留有效语句）
        QStringList sqlList = sql.split(";", Qt::SkipEmptyParts);
        writeLog("INFO", "建表脚本分割后语句数量：" + QString::number(sqlList.size()), "DATABASE");

        // 先执行建表语句，再执行索引语句（保证顺序）
        QStringList createTableStmts; // 建表语句
        QStringList createIndexStmts; // 索引语句

        for (int i = 0; i < sqlList.size(); i++) {
            QString rawStmt = sqlList[i];
            QString cleanStmt = cleanSqlStatement(rawStmt); // 清理注释和空白

            // 跳过清理后为空的语句
            if (cleanStmt.isEmpty()) {
                writeLog("INFO", QString("跳过建表语句%1：清理后为空").arg(i+1), "DATABASE");
                continue;
            }

            // 分类：建表/索引语句
            if (cleanStmt.startsWith("CREATE TABLE", Qt::CaseInsensitive) ||
                cleanStmt.startsWith("DROP TABLE", Qt::CaseInsensitive)) {
                createTableStmts.append(cleanStmt);
            } else if (cleanStmt.startsWith("CREATE INDEX", Qt::CaseInsensitive)) {
                createIndexStmts.append(cleanStmt);
            }

            // 执行当前语句（兼容原有逻辑，同时分类）
            if (!query.exec(cleanStmt)) {
                writeLog("WARNING", QString("建表语句%1执行失败：%2 | 清理后SQL：%3")
                         .arg(i+1).arg(query.lastError().text()).arg(cleanStmt.left(100)), "DATABASE");
            } else {
                writeLog("INFO", QString("建表语句%1执行成功 | 类型：%2")
                         .arg(i+1).arg(cleanStmt.startsWith("CREATE TABLE") ? "建表" : "索引"), "DATABASE");
            }
        }

        // 兜底：若索引语句执行失败，单独再执行一次（确保表已创建）
        if (!createIndexStmts.isEmpty()) {
            writeLog("INFO", "兜底执行索引创建语句，共" + QString::number(createIndexStmts.size()) + "条", "DATABASE");
            for (int i = 0; i < createIndexStmts.size(); i++) {
                if (!query.exec(createIndexStmts[i])) {
                    writeLog("WARNING", QString("兜底创建索引%1失败：%2").arg(i+1).arg(query.lastError().text()), "DATABASE");
                } else {
                    writeLog("INFO", QString("兜底创建索引%1成功").arg(i+1), "DATABASE");
                }
            }
        }

        writeLog("INFO", "从资源文件加载建表脚本成功", "DATABASE");
    } else {
        writeLog("WARNING", "未找到建表脚本，使用内置SQL", "DATABASE");

        // 备用内置建表SQL（保证先建表再建索引）
        QStringList builtInSql = {
            R"(DROP TABLE IF EXISTS course_schedule)",
            R"(DROP TABLE IF EXISTS class_info)",
            R"(DROP TABLE IF EXISTS notices)",
            R"(CREATE TABLE IF NOT EXISTS class_info (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                room_number TEXT NOT NULL UNIQUE,
                class_name TEXT NOT NULL,
                grade TEXT NOT NULL,
                department TEXT NOT NULL
            ))",
            R"(CREATE TABLE IF NOT EXISTS course_schedule (
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
            ))",
            R"(CREATE TABLE IF NOT EXISTS notices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                content TEXT NOT NULL,
                publish_time TEXT NOT NULL,
                expire_time TEXT,
                is_scrolling INTEGER DEFAULT 0,
                is_valid INTEGER DEFAULT 1
            ))",
            R"(CREATE INDEX IF NOT EXISTS idx_course_class_id ON course_schedule(class_id))",
            R"(CREATE INDEX IF NOT EXISTS idx_course_date ON course_schedule(start_date, end_date))",
            R"(CREATE INDEX IF NOT EXISTS idx_notices_valid ON notices(is_valid, expire_time))"
        };

        for (int i = 0; i < builtInSql.size(); i++) {
            if (!query.exec(builtInSql[i])) {
                writeLog("WARNING", QString("内置建表语句%1执行失败：%2").arg(i+1).arg(query.lastError().text()), "DATABASE");
            } else {
                writeLog("INFO", QString("内置建表语句%1执行成功").arg(i+1), "DATABASE");
            }
        }
    }

    // -------------------------- 导入测试数据（仅当无班级数据时） --------------------------
    // 先检查class_info是否已有数据
    query.exec("SELECT COUNT(*) FROM class_info");
    int classCount = 0;
    if (query.next()) {
        classCount = query.value(0).toInt();
    }
    writeLog("INFO", QString("检测到现有班级数据数量：%1").arg(classCount), "DATABASE");

    if (classCount > 0) {
        writeLog("INFO", "已有班级数据，跳过测试数据导入", "DATABASE");
        return;
    }

    QFile testSqlFile(":/sql/test_data.sql");
    writeLog("INFO", "尝试读取测试数据脚本：:/sql/test_data.sql", "DATABASE");

    if (testSqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 处理UTF-8 BOM
        QByteArray testContent = testSqlFile.readAll();
        if (testContent.startsWith("\xef\xbb\xbf")) {
            testContent = testContent.mid(3);
            writeLog("INFO", "测试数据脚本检测到UTF-8 BOM，已移除", "DATABASE");
        }
        QString testSql = QString::fromUtf8(testContent);
        testSqlFile.close();

        writeLog("INFO", "测试数据脚本总长度：" + QString::number(testSql.length()) + "字符", "DATABASE");
        writeLog("INFO", "测试数据脚本前300字符：" + testSql.left(300) + "...", "DATABASE");

        // 分割SQL语句（按分号分割）
        QStringList testSqlList = testSql.split(";", Qt::SkipEmptyParts);
        writeLog("INFO", "测试数据脚本分割后语句数量：" + QString::number(testSqlList.size()), "DATABASE");

        int successCount = 0;
        int failCount = 0;
        int skipCount = 0;

        for (int i = 0; i < testSqlList.size(); i++) {
            QString rawStmt = testSqlList[i];
            QString cleanStmt = cleanSqlStatement(rawStmt); // 清理注释和空白

            // 跳过清理后为空的语句
            if (cleanStmt.isEmpty()) {
                skipCount++;
                writeLog("INFO", QString("测试数据语句%1：清理后为空，跳过").arg(i+1), "DATABASE");
                continue;
            }

            // 执行清理后的有效SQL
            writeLog("INFO", QString("测试数据语句%1：执行SQL -> %2")
                     .arg(i+1).arg(cleanStmt.left(100) + (cleanStmt.length()>100 ? "..." : "")), "DATABASE");

            if (query.exec(cleanStmt)) {
                successCount++;
                writeLog("INFO", QString("测试数据语句%1：执行成功").arg(i+1), "DATABASE");
            } else {
                failCount++;
                writeLog("ERROR", QString("测试数据语句%1：执行失败 -> %2 | 清理后SQL：%3")
                         .arg(i+1).arg(query.lastError().text()).arg(cleanStmt.left(200)), "DATABASE");
            }
        }

        // 输出导入统计
        writeLog("INFO", QString("测试数据导入统计：成功%1条 | 失败%2条 | 跳过%3条")
                 .arg(successCount).arg(failCount).arg(skipCount), "DATABASE");

        if (failCount == 0 && successCount > 0) {
            writeLog("INFO", "从test_data.sql导入测试数据成功", "DATABASE");
        } else if (successCount == 0 && failCount == 0) {
            writeLog("ERROR", "测试数据导入失败：无有效可执行的SQL语句", "DATABASE");
        } else {
            writeLog("ERROR", "测试数据导入部分失败，需检查SQL语句", "DATABASE");
        }
    } else {
        writeLog("WARNING", "未找到test_data.sql，跳过测试数据导入 | 错误：" + testSqlFile.errorString(), "DATABASE");
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

    writeLog("INFO", "查询所有班级，数量：" + QString::number(classList.size()), "DATABASE");

    // 额外日志：输出查询到的班级列表
    if (classList.isEmpty()) {
        writeLog("WARNING", "查询所有班级：结果为空", "DATABASE");
    } else {
        QString classNames;
        for (const QVariantMap& cls : classList) {
            classNames += cls["class_name"].toString() + ",";
        }
        writeLog("INFO", "查询到的班级列表：" + classNames.left(classNames.length()-1), "DATABASE");
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
        writeLog("INFO", "搜索班级成功，关键词：" + keyword + "，数量：" + QString::number(classList.size()), "DATABASE");
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
        writeLog("INFO", "查询班级课程成功，班级ID：" + QString::number(classId) + "，数量：" + QString::number(courseList.size()), "DATABASE");
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
        writeLog("INFO", "查询当前课程成功，班级ID：" + QString::number(classId) + "，课程：" + currentCourse["course_name"].toString(), "DATABASE");
    } else {
        writeLog("INFO", "当前无课程，班级ID：" + QString::number(classId), "DATABASE");
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
        writeLog("INFO", "查询下节课成功，班级ID：" + QString::number(classId) + "，课程：" + nextCourse["course_name"].toString(), "DATABASE");
    } else {
        writeLog("INFO", "暂无下节课，班级ID：" + QString::number(classId), "DATABASE");
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
        writeLog("INFO", QString("更新通知状态成功，ID：%1，滚动：%2，有效：%3")
                 .arg(noticeId).arg(isScrolling).arg(isValid), "DATABASE");
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
        writeLog("INFO", QString("查询有效通知成功，滚动筛选：%1，数量：%2")
                 .arg(isScrolling).arg(noticeList.size()), "DATABASE");

        // 额外日志：输出通知列表
        if (noticeList.isEmpty()) {
            writeLog("WARNING", "查询有效通知：结果为空", "DATABASE");
        } else {
            QString noticeTitles;
            for (const QVariantMap& notice : noticeList) {
                noticeTitles += notice["title"].toString() + ",";
            }
            writeLog("INFO", "查询到的通知列表：" + noticeTitles.left(noticeTitles.length()-1), "DATABASE");
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
