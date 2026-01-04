#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVariantMap>
#include "data/DatabaseManager.h"
#include "ui/NoticeManager.h"
#include "ui/SettingsDialog.h"
#include "network/NetworkWorker.h"

namespace Ui {
class MainWindow; // Qt 6自动生成的UI类
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // 刷新UI（课程/通知/时间）
    void refreshUI();
    // 更新当前时间
    void updateCurrentTime();
    // 滚动通知
    void scrollMarquee();

private slots:
    // 班级选中事件
    void onClassSelected(const QModelIndex& index);
    // 班级筛选事件
    void onSearchTextChanged(const QString& text);
    // 按钮点击事件
    void on_settingsBtn_clicked();
    void on_noticeBtn_clicked();
    void on_exportBtn_clicked();

private:
    Ui::MainWindow *ui;                  // UI对象
    QSqlTableModel* m_classModel;        // 班级模型
    QSortFilterProxyModel* m_filterModel;// 筛选模型
    QTimer* m_timeTimer;                 // 时间刷新定时器
    QTimer* m_marqueeTimer;              // 滚动通知定时器
    QTimer* m_refreshTimer;              // UI刷新定时器
    int m_currentClassId = -1;           // 当前选中班级ID
    QList<QVariantMap> m_notices;        // 滚动通知列表
    int m_currentNoticeIndex = 0;        // 当前滚动通知索引
    int m_marqueeOffset = 0;             // 滚动偏移量
    NetworkWorker* m_networkWorker; // 网络同步对象

    // 初始化Model/View
    void initModel();
    // 初始化定时器
    void initTimer();
    // 加载滚动通知
    void loadMarqueeNotices();
    // 更新当前课程显示
    void updateCurrentCourse(const QVariantMap& course);
    // 更新下节课显示
    void updateNextCourse(const QVariantMap& course);
};

#endif // MAINWINDOW_H