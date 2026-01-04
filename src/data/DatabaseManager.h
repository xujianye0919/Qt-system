#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QVariantList>

// 单例模式：数据库管理类（Qt 6适配）
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // 单例实例获取
    static DatabaseManager& instance();
    ~DatabaseManager() = default;

    // 初始化数据库（返回是否成功）
    bool init(const QString& dbPath = "");

    // 获取数据库连接（供外部查询使用）
    QSqlDatabase getDb() const { return m_db; }

    // -------------------------- 班级管理 --------------------------
    // 添加班级
    bool addClass(const QString& roomNumber, const QString& className, const QString& grade, const QString& department);
    // 删除班级（按ID）
    bool deleteClass(int classId);
    // 查询所有班级
    QList<QVariantMap> getAllClasses();
    // 模糊搜索班级（按名称/教室编号）
    QList<QVariantMap> searchClasses(const QString& keyword);

    // -------------------------- 课表管理 --------------------------
    // 添加课程
    bool addCourse(int classId, const QString& courseName, const QString& teacher, const QString& courseType,
                   const QString& startTime, const QString& endTime, int dayOfWeek,
                   const QString& startDate, const QString& endDate, const QString& classroom = "");
    // 删除课程（按ID）
    bool deleteCourse(int courseId);
    // 查询班级的所有课程（含有效期过滤）
    QList<QVariantMap> getCoursesByClassId(int classId);
    // 查询班级当前正在上的课程
    QVariantMap getCurrentCourse(int classId);
    // 查询班级下一节课
    QVariantMap getNextCourse(int classId);

    // -------------------------- 通知管理 --------------------------
    // 添加通知
    bool addNotice(const QString& title, const QString& content, const QString& publishTime,
                   const QString& expireTime = "", bool isScrolling = false);
    // 删除通知（按ID）
    bool deleteNotice(int noticeId);
    // 更新通知状态（是否滚动/是否有效）
    bool updateNoticeStatus(int noticeId, bool isScrolling, bool isValid);
    // 查询所有有效通知（含过期过滤）
    QList<QVariantMap> getValidNotices(bool isScrolling = false);

signals:
    // 数据库操作结果通知
    void operateSuccess(const QString& msg);
    void operateFailed(const QString& msg);

private:
    // 私有构造函数（单例）
    DatabaseManager(QObject* parent = nullptr) : QObject(parent) {}
    // 禁止拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 创建数据表
    void createTables();
    // 检查日期是否在有效期内
    bool isDateInRange(const QString& checkDate, const QString& startDate, const QString& endDate);
    // 格式化日期字符串（统一为YYYY-MM-DD）
    QString formatDate(const QString& dateStr);

private:
    QSqlDatabase m_db;          // 数据库连接
    QString m_dbPath;           // 数据库路径
    const QString m_dbName = "classboard.db"; // 数据库文件名
};

#endif // DATABASEMANAGER_H