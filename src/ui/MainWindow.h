#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QPointer>
#include "data/DatabaseManager.h"
#include "network/NetworkWorker.h"
#include "settings/SettingsManager.h"
#include "utility/TimeHelper.h"
#include "utility/ExportHelper.h"
#include "ui/NoticeManager.h"
#include "ui/SettingsDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 界面交互槽函数
    void onClassSelected(int index);
    void onSearchTextChanged(const QString& text);
    void onExportBtnClicked();
    void onNoticeManagerBtnClicked();
    void onSettingsBtnClicked();

    // 定时器槽函数
    void updateCourseInfo();
    void updateMarqueeNotice();

    // 辅助槽函数
    void refreshUI();
    void onSyncSuccess(const QString& msg);
    void onSyncFailed(const QString& msg);

private:
    Ui::MainWindow *ui;

    // 数据模型
    QStandardItemModel* m_courseModel = nullptr;       // 课表模型
    QSortFilterProxyModel* m_filterModel = nullptr;    // 筛选模型

    // 状态变量
    int m_currentClassId = -1;               // 当前选中班级ID
    QString m_currentClassName = "";         // 当前选中班级名称

    // 定时器
    QTimer* m_courseTimer = nullptr;         // 课程信息更新定时器（1秒）
    QTimer* m_noticeTimer = nullptr;         // 通知滚动定时器（5秒）

    // 核心组件
    NetworkWorker* m_networkWorker = nullptr;  // 网络同步组件

    // 使用QPointer管理对话框，当对话框被删除时会自动设置为nullptr
    QPointer<NoticeManager> m_noticeManager;   // 通知管理窗口
    QPointer<SettingsDialog> m_settingsDialog; // 系统设置窗口

    // 辅助函数
    void initUI();                           // 初始化UI
    void initModels();                       // 初始化数据模型
    void initTimers();                       // 初始化定时器
    void loadClassList();                    // 加载班级列表
    void loadCourseTable(int classId);       // 加载班级课表
    void updateCurrentCourse(const QVariantMap& course); // 更新当前课程
    void updateNextCourse(const QVariantMap& course);     // 更新下节课
    void startMarquee(const QString& text);  // 启动通知滚动
};

#endif // MAINWINDOW_H
