#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 加载样式表（Qt 6 资源文件）
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
    emit m_networkWorker->startSyncTimer(); // 启动定时同步
    
    // 加载初始数据
    loadClassList();
    refreshUI();
    
    // 状态栏提示
    ui->statusBar->showMessage(QString("系统已就绪 - 当前时间：%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

MainWindow::~MainWindow()
{
    // 停止定时器
    if (m_courseTimer) m_courseTimer->stop();
    if (m_noticeTimer) m_noticeTimer->stop();
    
    delete ui;
    delete m_courseModel;
    delete m_filterModel;
    delete m_classModel;
    delete m_networkWorker;
}

// -------------------------- 初始化函数 --------------------------
void MainWindow::initUI()
{
    // 设置窗口属性
    this->setMinimumSize(1000, 700);
    this->setWindowIcon(QIcon(":/icons/app.ico")); // 可选：添加应用图标
    
    // 初始化子窗口
    m_noticeManager = new NoticeManager(this);
    m_settingsDialog = new SettingsDialog(this);
    
    // 连接子窗口信号
    connect(m_settingsDialog, &SettingsDialog::finished, this, [=]() {
        // 设置保存后刷新同步间隔
        m_networkWorker->setSyncInterval(SettingsManager::instance().getSyncInterval());
        refreshUI();
    });
}

void MainWindow::initModels()
{
    // 课表模型
    m_courseModel = new QStandardItemModel(this);
    QStringList courseHeaders = {"星期", "课程名称", "教师", "类型", "开始时间", "结束时间", "教室"};
    m_courseModel->setHorizontalHeaderLabels(courseHeaders);
    
    // 筛选模型（支持多列筛选）
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setSourceModel(m_courseModel);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 忽略大小写
    m_filterModel->setFilterKeyColumn(-1); // 筛选所有列
    
    // 绑定到TableView
    ui->classTableView->setModel(m_filterModel);
    ui->classTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->classTableView->verticalHeader()->setVisible(false);
    
    // 班级下拉框模型
    m_classModel = new QStandardItemModel(this);
    m_classModel->setHorizontalHeaderLabels({"班级ID", "班级名称", "教室", "院系"});
    ui->classComboBox->setModel(m_classModel);
    ui->classComboBox->setModelColumn(1); // 显示班级名称列
}

void MainWindow::initTimers()
{
    // 课程信息更新定时器（1秒刷新一次）
    m_courseTimer = new QTimer(this);
    m_courseTimer->setInterval(1000);
    connect(m_courseTimer, &QTimer::timeout, this, &MainWindow::updateCourseInfo);
    m_courseTimer->start();
    
    // 通知滚动定时器（5秒刷新一次）
    m_noticeTimer = new QTimer(this);
    m_noticeTimer->setInterval(5000);
    connect(m_noticeTimer, &QTimer::timeout, this, &MainWindow::updateMarqueeNotice);
    m_noticeTimer->start();
}

// -------------------------- 界面交互槽函数 --------------------------
void MainWindow::onClassSelected(int index)
{
    if (index < 0 || m_classModel->rowCount() <= index) {
        m_currentClassId = -1;
        m_currentClassName = "";
        return;
    }
    
    // 获取选中班级ID
    QStandardItem* idItem = m_classModel->item(index, 0);
    if (idItem) {
        m_currentClassId = idItem->text().toInt();
        m_currentClassName = m_classModel->item(index, 1)->text();
        
        // 加载该班级课表
        loadCourseTable(m_currentClassId);
        
        // 更新状态栏
        ui->statusBar->showMessage(QString("已选中：%1（ID：%2）").arg(m_currentClassName).arg(m_currentClassId));
    }
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    // 多列模糊筛选
    m_filterModel->setFilterFixedString(text);
    
    // 筛选后自动选中第一行（如果有数据）
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
    
    // 导出当前班级课表
    QList<QVariantMap> courses = DatabaseManager::instance().getCoursesByClassId(m_currentClassId);
    bool success = ExportHelper::exportCoursesToExcel(courses, QString("%1课表").arg(m_currentClassName));
    
    if (!success) {
        QMessageBox::critical(this, "失败", "课表导出失败！\n请确认：\n1. 已安装Microsoft Office\n2. 有桌面写入权限");
    }
}

void MainWindow::onNoticeManagerBtnClicked()
{
    m_noticeManager->refreshNoticeList(); // 刷新数据
    m_noticeManager->exec();              // 模态显示
    updateMarqueeNotice();                // 关闭后更新通知
}

void MainWindow::onSettingsBtnClicked()
{
    m_settingsDialog->exec(); // 模态显示设置窗口
}

// -------------------------- 定时器槽函数 --------------------------
void MainWindow::updateCourseInfo()
{
    if (m_currentClassId == -1) return;
    
    // 更新当前课程
    QVariantMap currentCourse = DatabaseManager::instance().getCurrentCourse(m_currentClassId);
    updateCurrentCourse(currentCourse);
    
    // 更新下节课
    QVariantMap nextCourse = DatabaseManager::instance().getNextCourse(m_currentClassId);
    updateNextCourse(nextCourse);
    
    // 更新状态栏时间
    ui->statusBar->showMessage(QString("系统已就绪 - 当前时间：%1 | 选中：%2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                               .arg(m_currentClassName));
}

void MainWindow::updateMarqueeNotice()
{
    // 获取滚动通知
    QList<QVariantMap> scrollNotices = DatabaseManager::instance().getValidNotices(true);
    
    if (scrollNotices.isEmpty()) {
        ui->marqueeLabel->setText("欢迎使用教室班牌信息展示系统 - 暂无滚动通知");
        return;
    }
    
    // 轮播显示通知
    static int noticeIndex = 0;
    if (noticeIndex >= scrollNotices.size()) noticeIndex = 0;
    
    QVariantMap notice = scrollNotices[noticeIndex];
    QString noticeText = QString("[%1] %2：%3")
                         .arg(notice["publish_time"].toString().left(10))
                         .arg(notice["title"].toString())
                         .arg(notice["content"].toString());
    
    // 启动滚动效果
    startMarquee(noticeText);
    noticeIndex++;
}

// -------------------------- 辅助函数 --------------------------
void MainWindow::refreshUI()
{
    // 重新加载班级列表
    loadClassList();
    
    // 刷新课程信息
    if (m_currentClassId != -1) {
        loadCourseTable(m_currentClassId);
        updateCourseInfo();
    }
    
    // 刷新通知
    updateMarqueeNotice();
}

void MainWindow::loadClassList()
{
    // 清空班级模型
    m_classModel->clear();
    m_classModel->setHorizontalHeaderLabels({"班级ID", "班级名称", "教室", "院系"});
    
    // 加载所有班级
    QList<QVariantMap> classes = DatabaseManager::instance().getAllClasses();
    
    for (const QVariantMap& cls : classes) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(cls["id"].toInt())));
        items.append(new QStandardItem(cls["class_name"].toString()));
        items.append(new QStandardItem(cls["room_number"].toString()));
        items.append(new QStandardItem(cls["department"].toString()));
        
        // 设置项不可编辑
        for (QStandardItem* item : items) {
            item->setEditable(false);
        }
        
        m_classModel->appendRow(items);
    }
    
    // 默认选中第一个班级
    if (m_classModel->rowCount() > 0) {
        ui->classComboBox->setCurrentIndex(0);
        m_currentClassId = m_classModel->item(0, 0)->text().toInt();
        m_currentClassName = m_classModel->item(0, 1)->text();
        loadCourseTable(m_currentClassId);
    }
}

void MainWindow::loadCourseTable(int classId)
{
    // 清空课表模型
    m_courseModel->clear();
    QStringList courseHeaders = {"星期", "课程名称", "教师", "类型", "开始时间", "结束时间", "教室"};
    m_courseModel->setHorizontalHeaderLabels(courseHeaders);
    
    // 加载班级课表
    QList<QVariantMap> courses = DatabaseManager::instance().getCoursesByClassId(classId);
    
    for (const QVariantMap& course : courses) {
        QList<QStandardItem*> items;
        
        // 星期转换（1-7 → 周一-周日）
        QStringList weekDays = { "", "周一", "周二", "周三", "周四", "周五", "周六", "周日" };
        int dayOfWeekInt = course["day_of_week"].toInt();
        QString dayOfWeek = (dayOfWeekInt >= 1 && dayOfWeekInt <=7) ? weekDays[dayOfWeekInt] : "未知";
        
        items.append(new QStandardItem(dayOfWeek));
        items.append(new QStandardItem(course["course_name"].toString()));
        items.append(new QStandardItem(course["teacher"].toString()));
        items.append(new QStandardItem(course["course_type"].toString()));
        items.append(new QStandardItem(course["start_time"].toString()));
        items.append(new QStandardItem(course["end_time"].toString()));
        items.append(new QStandardItem(course["classroom"].toString()));
        
        // 设置项不可编辑
        for (QStandardItem* item : items) {
            item->setEditable(false);
            // 标记当前课程
            if (TimeHelper::isTimeInRange(course["start_time"].toString(), course["end_time"].toString())) {
                item->setBackground(QColor(255, 240, 240)); // 浅红背景
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
    ui->currentCourseTime->setText(QString("上课时间：%1 至 %2")
                                   .arg(course["start_time"].toString(), course["end_time"].toString()));
    
    // 计算倒计时
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
    ui->nextCourseTime->setText(QString("上课时间：%1 至 %2")
                                   .arg(course["start_time"].toString(), course["end_time"].toString()));
}

void MainWindow::startMarquee(const QString& text)
{
    // 简单滚动效果（Qt 6 纯代码实现）
    static QString fullText = "";
    static int pos = 0;
    
    fullText = text;
    pos = 0;
    
    // 启动单次定时器实现滚动
    QTimer* marqueeTimer = new QTimer(this);
    marqueeTimer->setInterval(100);
    connect(marqueeTimer, &QTimer::timeout, this, [=]() {
        if (pos > fullText.length()) {
            pos = 0;
        }
        
        QString displayText = fullText.mid(pos) + "  " + fullText.left(pos);
        ui->marqueeLabel->setText(displayText);
        pos++;
        
        // 防止内存泄漏
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
    refreshUI(); // 同步成功后刷新UI
}

void MainWindow::onSyncFailed(const QString& msg)
{
    ui->statusBar->showMessage(QString("数据同步失败：%1 | 当前时间：%2")
                               .arg(msg).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    QMessageBox::warning(this, "同步警告", msg);
}
