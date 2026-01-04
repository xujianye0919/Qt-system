#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "utility/TimeHelper.h"
#include "utility/ExportHelper.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // 加载UI（Qt 6自动生成的ui_MainWindow.h）
    ui->setupUi(this);

    // 初始化数据库（单例）
    DatabaseManager::instance().init();

    // 初始化Model/View
    initModel();

    // 初始化定时器
    initTimer();

    // 加载滚动通知
    loadMarqueeNotices();

    // 绑定信号槽
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(ui->classTableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onClassSelected);

    // 默认选中第一个班级
    if (m_filterModel->rowCount() > 0) {
        ui->classTableView->selectRow(0);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化Model/View
void MainWindow::initModel()
{
    // 班级模型（关联数据库表）
    m_classModel = new QSqlTableModel(this, DatabaseManager::instance().getDb());
    m_classModel->setTable("class_info");
    m_classModel->setHeaderData(1, Qt::Horizontal, "教室编号");
    m_classModel->setHeaderData(2, Qt::Horizontal, "班级名称");
    m_classModel->setHeaderData(3, Qt::Horizontal, "年级");
    m_classModel->setHeaderData(4, Qt::Horizontal, "院系");
    m_classModel->select();

    // 筛选模型（Qt 6支持）
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setSourceModel(m_classModel);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterRole(Qt::DisplayRole);

    // 绑定到TableView
    ui->classTableView->setModel(m_filterModel);
    ui->classTableView->hideColumn(0); // 隐藏ID列
    ui->classTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->classTableView->verticalHeader()->setVisible(false);
}

// 初始化定时器
void MainWindow::initTimer()
{
    // 时间刷新定时器（1秒一次）
    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, &QTimer::timeout, this, &MainWindow::updateCurrentTime);
    m_timeTimer->start(1000);
    updateCurrentTime();

    // 滚动通知定时器（50毫秒一次）
    m_marqueeTimer = new QTimer(this);
    connect(m_marqueeTimer, &QTimer::timeout, this, &MainWindow::scrollMarquee);
    m_marqueeTimer->start(50);

    // UI刷新定时器（30秒一次）
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshUI);
    m_refreshTimer->start(30000);
}

// 刷新UI
void MainWindow::refreshUI()
{
    if (m_currentClassId == -1) return;

    // 刷新当前课程
    QVariantMap currentCourse = DatabaseManager::instance().getCurrentCourse(m_currentClassId);
    updateCurrentCourse(currentCourse);

    // 刷新下节课
    QVariantMap nextCourse = DatabaseManager::instance().getNextCourse(m_currentClassId);
    updateNextCourse(nextCourse);

    // 刷新滚动通知
    loadMarqueeNotices();
}

// 更新当前时间
void MainWindow::updateCurrentTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    ui->timeLabel->setText(currentTime);

    // 刷新倒计时（如果有当前课程）
    if (m_currentClassId != -1) {
        QVariantMap currentCourse = DatabaseManager::instance().getCurrentCourse(m_currentClassId);
        if (!currentCourse.isEmpty()) {
            QString endTime = currentCourse["end_time"].toString();
            QString countdown = TimeHelper::getCountdown(endTime);
            ui->countdownLabel->setText(QString("倒计时：%1").arg(countdown));
        }
    }
}

// 加载滚动通知
void MainWindow::loadMarqueeNotices()
{
    m_notices = DatabaseManager::instance().getValidNotices(true);
    m_currentNoticeIndex = 0;
    m_marqueeOffset = 0;
}

// 滚动通知
void MainWindow::scrollMarquee()
{
    if (m_notices.isEmpty()) {
        ui->marqueeLabel->setText("欢迎使用教室班牌信息展示系统 - 暂无滚动通知");
        return;
    }

    // 获取当前通知
    QVariantMap notice = m_notices[m_currentNoticeIndex];
    QString content = QString("[%1] %2").arg(notice["title"].toString(), notice["content"].toString());
    QString displayText = content + "   "; // 添加间隔

    // 滚动偏移
    m_marqueeOffset++;
    if (m_marqueeOffset > displayText.length()) {
        m_marqueeOffset = 0;
        m_currentNoticeIndex = (m_currentNoticeIndex + 1) % m_notices.size();
    }

    // 截取显示文本
    QString scrollText = displayText.mid(m_marqueeOffset) + displayText.left(m_marqueeOffset);
    ui->marqueeLabel->setText(scrollText);
}

// 班级选中事件
void MainWindow::onClassSelected(const QModelIndex& index)
{
    if (!index.isValid()) return;

    // 获取选中班级ID（Qt 6的index映射）
    QModelIndex sourceIndex = m_filterModel->mapToSource(index);
    m_currentClassId = m_classModel->data(m_classModel->index(sourceIndex.row(), 0)).toInt();

    // 刷新UI
    refreshUI();
}

// 班级筛选事件
void MainWindow::onSearchTextChanged(const QString& text)
{
    // 多列模糊筛选（匹配班级名称/教室编号/院系）
    m_filterModel->setFilterKeyColumn(-1); // -1表示筛选所有列
    m_filterModel->setFilterFixedString(text); // 模糊匹配
    // 筛选后自动选中第一行（如果有数据）
    if (m_filterModel->rowCount() > 0 && !ui->classTableView->selectionModel()->hasSelection()) {
        ui->classTableView->selectRow(0);
    }
}

// 更新当前课程显示
void MainWindow::updateCurrentCourse(const QVariantMap& course)
{
    if (course.isEmpty()) {
        ui->currentCourseName->setText("暂无课程");
        ui->currentCourseTeacher->setText("任课教师：暂无");
        ui->currentCourseTime->setText("上课时间：--:-- 至 --:--");
        ui->countdownLabel->setText("倒计时：--:--:--");
        return;
    }

    ui->currentCourseName->setText(course["course_name"].toString());
    ui->currentCourseTeacher->setText(QString("任课教师：%1").arg(course["teacher"].toString()));
    ui->currentCourseTime->setText(QString("上课时间：%1 至 %2")
                                   .arg(course["start_time"].toString(), course["end_time"].toString()));

    // 计算倒计时
    QString endTime = course["end_time"].toString();
    QString countdown = TimeHelper::getCountdown(endTime);
    ui->countdownLabel->setText(QString("倒计时：%1").arg(countdown));
}

// 更新下节课显示
void MainWindow::updateNextCourse(const QVariantMap& course)
{
    if (course.isEmpty()) {
        ui->nextCourseInfo->setText("课程名称：暂无 | 上课时间：--:-- | 教室：暂无");
        return;
    }

    ui->nextCourseInfo->setText(QString("课程名称：%1 | 上课时间：%2 | 教室：%3")
                                 .arg(course["course_name"].toString())
                                 .arg(course["start_time"].toString())
                                 .arg(course["classroom"].toString().isEmpty() ? "无" : course["classroom"].toString()));
}

// 设置按钮点击
void on_settingsBtn_clicked()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

// 通知管理按钮点击
void on_noticeBtn_clicked()
{
    NoticeManager dialog(this);
    dialog.exec();
}

// 导出数据按钮点击
void on_exportBtn_clicked()
{
    if (m_currentClassId == -1) {
        QMessageBox::warning(this, "警告", "请先选择班级！");
        return;
    }

    // 导出当前班级课表
    QList<QVariantMap> courses = DatabaseManager::instance().getCoursesByClassId(m_currentClassId);
    bool success = ExportHelper::exportCoursesToExcel(courses, QString("班级%1课表").arg(m_currentClassId));

    if (success) {
        QMessageBox::information(this, "成功", "课表导出成功！");
    } else {
        QMessageBox::critical(this, "失败", "课表导出失败！");
    }
}