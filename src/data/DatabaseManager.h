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
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QThread>
#include "utility/LogHelper.h" // 包含公共日志头文件

// 单例模式：数据库管理类（Qt 6.9.2适配）
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // 单例实例获取
    static DatabaseManager& instance();
    ~DatabaseManager() = default;

    // 初始化数据库（返回是否成功）
    bool init(const QString& dbPath = "");

    // 获取数据库连接
    QSqlDatabase getDb() const { return m_db; }

    // -------------------------- 班级管理 --------------------------
    // 移除 roomNumber 参数（适配新表结构，class_info 表无该字段）
    bool addClass(const QString& className, const QString& grade, const QString& department);
    bool deleteClass(int classId);
    QList<QVariantMap> getAllClasses();
    QList<QVariantMap> searchClasses(const QString& keyword);

    // -------------------------- 教室管理（新增，适配 classroom_info 表） --------------------------
    bool addClassroom(const QString& classroomName);          // 添加教室
    QList<QVariantMap> getAllClassrooms();                    // 获取所有教室
    QString getClassroomNameById(int classroomId);            // 根据ID获取教室名称

    // -------------------------- 课表管理 --------------------------
    // 修改：classroom 改为 classroomId（int类型，关联教室表主键）
    bool addCourse(int classId, const QString& courseName, const QString& teacher, const QString& courseType,
                   const QString& startTime, const QString& endTime, int dayOfWeek,
                   const QString& startDate, const QString& endDate, int classroomId = 0);
    bool deleteCourse(int courseId);
    QList<QVariantMap> getCoursesByClassId(int classId);
    QVariantMap getCurrentCourse(int classId);
    QVariantMap getNextCourse(int classId);

    // -------------------------- 通知管理 --------------------------
    bool addNotice(const QString& title, const QString& content, const QString& publishTime,
                   const QString& expireTime = "", bool isScrolling = false);
    bool deleteNotice(int noticeId);
    bool updateNoticeStatus(int noticeId, bool isScrolling, bool isValid);
    QList<QVariantMap> getValidNotices(bool isScrolling = false);

signals:
    void operateSuccess(const QString& msg);
    void operateFailed(const QString& msg);

private:
    DatabaseManager(QObject* parent = nullptr) : QObject(parent) {}
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 内部方法
    void createTables();
    bool isDateInRange(const QString& checkDate, const QString& startDate, const QString& endDate);
    QString formatDate(const QString& dateStr);
    QString cleanSqlStatement(const QString& stmt);

private:
    QSqlDatabase m_db;          // 数据库连接
    QString m_dbPath;           // 数据库路径
    const QString m_dbName = "classboard.db"; // 数据库文件名
};

#endif // DATABASEMANAGER_H
