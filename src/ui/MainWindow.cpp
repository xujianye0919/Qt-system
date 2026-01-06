#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFile>
#include <QIcon>
#include <QHeaderView>
#include <QStandardItem>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_courseModel(nullptr)
    , m_filterModel(nullptr)
    , m_courseTimer(nullptr)
    , m_noticeTimer(nullptr)
    , m_networkWorker(nullptr)
{
    ui->setupUi(this);

    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = styleFile.readAll();
        this->setStyleSheet(styleSheet);
        styleFile.close();
    }

    // 初始化数据库
    DatabaseManager::instance().init(SettingsManager::instance().getDbPath());

    // 初始化UI/模型/定时器
    initUI();
    initModels();
    initTimers();

    // 初始化网络同步
    m_networkWorker = new NetworkWorker(this);
    m_networkWorker->setSyncInterval(SettingsManager::instance().getSyncInterval());
    connect(m_networkWorker, &NetworkWorker::syncSuccess, this, &MainWindow::onSyncSuccess);
    connect(m_networkWorker, &NetworkWorker::syncFailed, this, &MainWindow::onSyncFailed);
    emit m_networkWorker->startSyncTimer();

    // 加载初始数据
    loadClassList();
    refreshUI();

    // 状态栏提示
    ui->statusBar->showMessage(QString("系统已就绪 - 当前时间：%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

MainWindow::~MainWindow()
{
    if (m_courseTimer) {
        m_courseTimer->stop();
        delete m_courseTimer;
    }

    if (m_noticeTimer) {
        m_noticeTimer->stop();
        delete m_noticeTimer;
    }

    delete ui;
    delete m_courseModel;
    delete m_filterModel;
    delete m_networkWorker;

    // QPointer会自动管理，不需要手动删除
}

// -------------------------- 初始化函数 --------------------------
void MainWindow::initUI()
{
    // 窗口属性
    this->setMinimumSize(1000, 700);
    this->setWindowIcon(QIcon(":/icons/app.ico"));

    // 初始化子窗口 - 不在这里创建，改为在需要时创建

    // 下拉框信号
    connect(ui->classComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onClassSelected);

    // 搜索框信号
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);

    // 按钮信号
    connect(ui->exportBtn, &QPushButton::clicked, this, &MainWindow::onExportBtnClicked);
    connect(ui->noticeManagerBtn, &QPushButton::clicked, this, &MainWindow::onNoticeManagerBtnClicked);
    connect(ui->settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsBtnClicked);
}

void MainWindow::initModels()
{
    // 课表模型
    m_courseModel = new QStandardItemModel(this);
    QStringList courseHeaders = {"星期", "课程名称", "教师", "类型", "开始时间", "结束时间", "教室"};
    m_courseModel->setHorizontalHeaderLabels(courseHeaders);

    // 筛选模型
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setSourceModel(m_courseModel);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterKeyColumn(-1);

    // 绑定TableView
    ui->classTableView->setModel(m_filterModel);
    ui->classTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->classTableView->verticalHeader()->setVisible(false);

    // 清空下拉框默认模型
    ui->classComboBox->clear();
    ui->classComboBox->setEditable(false);
    ui->classComboBox->setEnabled(false);
}

void MainWindow::initTimers()
{
    // 课程定时器（1秒）
    m_courseTimer = new QTimer(this);
    m_courseTimer->setInterval(1000);
    connect(m_courseTimer, &QTimer::timeout, this, &MainWindow::updateCourseInfo);
    m_courseTimer->start();

    // 通知定时器（10秒）
    m_noticeTimer = new QTimer(this);
    m_noticeTimer->setInterval(10000);
    connect(m_noticeTimer, &QTimer::timeout, this, &MainWindow::updateMarqueeNotice);
    m_noticeTimer->start();
}

// -------------------------- 界面交互槽函数 --------------------------
void MainWindow::onClassSelected(int index)
{
    // 容错：索引无效
    if (index < 0 || index >= ui->classComboBox->count()) {
        m_currentClassId = -1;
        m_currentClassName = "";
        m_courseModel->clear();
        ui->statusBar->showMessage("未选中任何班级 - 当前时间：" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        return;
    }

    // 取班级ID和名称
    m_currentClassId = ui->classComboBox->itemData(index).toInt();
    m_currentClassName = ui->classComboBox->itemText(index);

    // 加载课表
    loadCourseTable(m_currentClassId);
    // 立即更新课程信息
    updateCourseInfo();

    // 更新状态栏
    ui->statusBar->showMessage(QString("已选中：%1（ID：%2） - 当前时间：%3")
                               .arg(m_currentClassName)
                               .arg(m_currentClassId)
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    m_filterModel->setFilterFixedString(text);
    if (m_filterModel->rowCount() > 0 && !ui->classTableView->selectionModel()->hasSelection()) {
        ui->classTableView->selectRow(0);
    }
}

void MainWindow::onExportBtnClicked()
{
    if (m_currentClassId == -1) {
        QMessageBox::warning(this, "警告", "请先选择班级！");
        return;
    }

    QList<QVariantMap> courses = DatabaseManager::instance().getCoursesByClassId(m_currentClassId);
    bool success = ExportHelper::exportCoursesToExcel(courses, QString("%1课表").arg(m_currentClassName));

    if (success) {
        QMessageBox::information(this, "成功", QString("%1课表导出成功！").arg(m_currentClassName));
    } else {
        QMessageBox::critical(this, "失败", "课表导出失败！\n请确认：\n1. 已安装Microsoft Office\n2. 有桌面写入权限");
    }
}

void MainWindow::onNoticeManagerBtnClicked()
{
    // 如果对话框不存在或已关闭，创建新实例
    if (!m_noticeManager || !m_noticeManager->isVisible()) {
        m_noticeManager = new NoticeManager(this);
        // 设置属性，确保关闭时删除
        m_noticeManager->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_noticeManager->show();
    m_noticeManager->raise();  // 提升到最前
    m_noticeManager->activateWindow();  // 激活窗口
}

void MainWindow::onSettingsBtnClicked()
{
    // 如果对话框不存在或已关闭，创建新实例
    if (!m_settingsDialog || !m_settingsDialog->isVisible()) {
        m_settingsDialog = new SettingsDialog(this);
        // 设置属性，确保关闭时删除
        m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_settingsDialog->show();
    m_settingsDialog->raise();  // 提升到最前
    m_settingsDialog->activateWindow();  // 激活窗口
}

// -------------------------- 定时器槽函数 --------------------------
void MainWindow::updateCourseInfo()
{
    if (m_currentClassId == -1) return;

    QVariantMap currentCourse = DatabaseManager::instance().getCurrentCourse(m_currentClassId);
    updateCurrentCourse(currentCourse);

    QVariantMap nextCourse = DatabaseManager::instance().getNextCourse(m_currentClassId);
    updateNextCourse(nextCourse);

    ui->statusBar->showMessage(QString("系统已就绪 - 当前时间：%1 | 选中：%2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                               .arg(m_currentClassName));
}

void MainWindow::updateMarqueeNotice()
{
    QList<QVariantMap> scrollNotices = DatabaseManager::instance().getValidNotices(true);

    if (scrollNotices.isEmpty()) {
        ui->marqueeLabel->setText("欢迎使用教室班牌信息展示系统 - 暂无滚动通知");
        return;
    }

    static int noticeIndex = 0;
    if (noticeIndex >= scrollNotices.size()) noticeIndex = 0;

    QVariantMap notice = scrollNotices[noticeIndex];
    QString noticeText = QString("[%1] %2：%3")
                         .arg(notice["publish_time"].toString().left(10))
                         .arg(notice["title"].toString())
                         .arg(notice["content"].toString());

    startMarquee(noticeText);
    noticeIndex++;
}

// -------------------------- 辅助函数 --------------------------
void MainWindow::refreshUI()
{
    loadClassList();

    if (m_currentClassId != -1) {
        loadCourseTable(m_currentClassId);
        updateCourseInfo();
    }

    updateMarqueeNotice();
}

void MainWindow::loadClassList()
{
    // 清空下拉框
    ui->classComboBox->clear();

    // 从数据库取班级列表
    QList<QVariantMap> classes = DatabaseManager::instance().getAllClasses();

    // 手动添加每个班级到下拉框
    for (const QVariantMap& cls : classes) {
        int classId = cls["id"].toInt();
        QString className = cls["class_name"].toString();
        ui->classComboBox->addItem(className, classId);
    }

    // 启用下拉框
    ui->classComboBox->setEnabled(ui->classComboBox->count() > 0);

    // 默认选中第一个班级
    if (ui->classComboBox->count() > 0) {
        ui->classComboBox->setCurrentIndex(0);
    } else {
        m_currentClassId = -1;
        m_currentClassName = "";
        m_courseModel->clear();
        ui->statusBar->showMessage("暂无班级数据 - 当前时间：" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    }
}

void MainWindow::loadCourseTable(int classId)
{
    m_courseModel->clear();
    QStringList courseHeaders = {"星期", "课程名称", "教师", "类型", "开始时间", "结束时间", "教室"};
    m_courseModel->setHorizontalHeaderLabels(courseHeaders);

    QList<QVariantMap> courses = DatabaseManager::instance().getCoursesByClassId(classId);

    for (const QVariantMap& course : courses) {
        QList<QStandardItem*> items;

        // 星期转换
        QStringList weekDays = { "", "周一", "周二", "周三", "周四", "周五", "周六", "周日" };
        int dayOfWeekInt = course["day_of_week"].toInt();
        QString dayOfWeek = (dayOfWeekInt >= 1 && dayOfWeekInt <=7) ? weekDays[dayOfWeekInt] : "未知";

        items.append(new QStandardItem(dayOfWeek));
        items.append(new QStandardItem(course["course_name"].toString()));
        items.append(new QStandardItem(course["teacher"].toString()));
        items.append(new QStandardItem(course["course_type"].toString()));
        items.append(new QStandardItem(course["start_time"].toString()));
        items.append(new QStandardItem(course["end_time"].toString()));
        QString classroomName = course["classroom_name"].toString();
                if (classroomName.isEmpty()) {
                    classroomName = "未分配";
                }
        items.append(new QStandardItem(classroomName));

        for (QStandardItem* item : items) {
            item->setEditable(false);
            if (TimeHelper::isTimeInRange(course["start_time"].toString(), course["end_time"].toString())) {
                item->setBackground(QColor(255, 240, 240));
            }
        }

        m_courseModel->appendRow(items);
    }
}

void MainWindow::updateCurrentCourse(const QVariantMap& course)
{
    if (course.isEmpty()) {
        ui->currentCourseName->setText("暂无课程");
        ui->currentCourseTeacher->setText("任课教师：暂无");
        ui->currentCourseTime->setText("上课时间：--:-- 至 --:--");
        ui->countdownLabel->setText("倒计时：00:00:00");
        return;
    }

    ui->currentCourseName->setText(course["course_name"].toString());
    ui->currentCourseTeacher->setText(QString("任课教师：%1").arg(course["teacher"].toString()));
    QString classroomInfo = course["classroom_name"].toString();
        if (!classroomInfo.isEmpty()) {
            ui->currentCourseTeacher->setText(ui->currentCourseTeacher->text() + QString(" | 教室：%1").arg(classroomInfo));
        }
    ui->currentCourseTime->setText(QString("上课时间：%1 至 %2")
                                   .arg(course["start_time"].toString(), course["end_time"].toString()));

    QString countdown = TimeHelper::getCountdown(course["end_time"].toString());
    ui->countdownLabel->setText(QString("倒计时：%1").arg(countdown));
}

void MainWindow::updateNextCourse(const QVariantMap& course)
{
    if (course.isEmpty()) {
        ui->nextCourseName->setText("暂无课程");
        ui->nextCourseTeacher->setText("任课教师：暂无");
        ui->nextCourseTime->setText("上课时间：--:-- 至 --:--");
        return;
    }

    ui->nextCourseName->setText(course["course_name"].toString());
    ui->nextCourseTeacher->setText(QString("任课教师：%1").arg(course["teacher"].toString()));
    QString classroomInfo = course["classroom_name"].toString();
        if (!classroomInfo.isEmpty()) {
            ui->nextCourseTeacher->setText(ui->nextCourseTeacher->text() + QString(" | 教室：%1").arg(classroomInfo));
        }
    ui->nextCourseTime->setText(QString("上课时间：%1 至 %2")
                                   .arg(course["start_time"].toString(), course["end_time"].toString()));
}

void MainWindow::startMarquee(const QString& text)
{
    static QString fullText = "";
    static int pos = 0;

    fullText = text;
    pos = 0;

    QTimer* marqueeTimer = new QTimer(this);
    marqueeTimer->setInterval(500); // 滚动速度（已调整为适中）
    connect(marqueeTimer, &QTimer::timeout, this, [=]() {
        if (pos > fullText.length()) {
            pos = 0;
        }

        QString displayText = fullText.mid(pos) + "  " + fullText.left(pos);
        ui->marqueeLabel->setText(displayText);
        pos++;

        if (pos > fullText.length() + 10) {
            marqueeTimer->stop();
            marqueeTimer->deleteLater();
        }
    });
    marqueeTimer->start();
}

// -------------------------- 网络同步回调 --------------------------
void MainWindow::onSyncSuccess(const QString& msg)
{
    ui->statusBar->showMessage(QString("数据同步成功：%1 | 当前时间：%2")
                               .arg(msg).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    refreshUI();
}

void MainWindow::onSyncFailed(const QString& msg)
{
    ui->statusBar->showMessage(QString("数据同步失败：%1 | 当前时间：%2")
                               .arg(msg).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    QMessageBox::warning(this, "同步警告", msg);
}
