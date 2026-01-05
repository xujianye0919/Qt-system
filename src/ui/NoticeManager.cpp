#include "NoticeManager.h"
#include "ui_NoticeManager.h"

NoticeManager::NoticeManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NoticeManager)
{
    ui->setupUi(this);

    // 初始化通知模型
    m_noticeModel = new QSqlTableModel(this, DatabaseManager::instance().getDb());
    m_noticeModel->setTable("notices");

    // 设置表头
    m_noticeModel->setHeaderData(0, Qt::Horizontal, "ID");
    m_noticeModel->setHeaderData(1, Qt::Horizontal, "标题");
    m_noticeModel->setHeaderData(2, Qt::Horizontal, "内容");
    m_noticeModel->setHeaderData(3, Qt::Horizontal, "发布时间");
    m_noticeModel->setHeaderData(4, Qt::Horizontal, "过期时间");
    m_noticeModel->setHeaderData(5, Qt::Horizontal, "是否滚动");
    m_noticeModel->setHeaderData(6, Qt::Horizontal, "是否有效");

    // 按发布时间降序排序
    m_noticeModel->setSort(3, Qt::DescendingOrder);
    m_noticeModel->select();

    // 绑定到TableView
    ui->noticeTableView->setModel(m_noticeModel);
    ui->noticeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->noticeTableView->verticalHeader()->setVisible(false);

    // 设置窗口属性
    this->setModal(true);
    this->setMinimumSize(700, 500);
}

NoticeManager::~NoticeManager()
{
    delete ui;
    delete m_noticeModel;
}

// -------------------------- 核心函数 --------------------------
void NoticeManager::refreshNoticeList()
{
    m_noticeModel->select(); // 重新查询数据库
}

// -------------------------- 按钮点击事件 --------------------------
void NoticeManager::on_addBtn_clicked()
{
    showAddNoticeDialog();
}

void NoticeManager::on_deleteBtn_clicked()
{
    // 获取选中行
    QModelIndex index = ui->noticeTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "警告", "请先选中一条通知！");
        return;
    }

    int noticeId = m_noticeModel->data(m_noticeModel->index(index.row(), 0)).toInt();
    QString noticeTitle = m_noticeModel->data(m_noticeModel->index(index.row(), 1)).toString();

    if (QMessageBox::question(this, "确认删除", QString("是否删除通知「%1」（ID：%2）？")
                              .arg(noticeTitle).arg(noticeId)) == QMessageBox::Yes) {
        bool success = DatabaseManager::instance().deleteNotice(noticeId);
        if (success) {
            QMessageBox::information(this, "成功", "通知删除成功！");
            refreshNoticeList();
        } else {
            QMessageBox::critical(this, "失败", "通知删除失败！");
        }
    }
}

void NoticeManager::on_scrollBtn_clicked()
{
    // 获取选中行
    QModelIndex index = ui->noticeTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "警告", "请先选中一条通知！");
        return;
    }

    int noticeId = m_noticeModel->data(m_noticeModel->index(index.row(), 0)).toInt();
    bool isScrolling = m_noticeModel->data(m_noticeModel->index(index.row(), 5)).toInt() == 1;

    // 更新滚动状态
    bool success = DatabaseManager::instance().updateNoticeStatus(noticeId, !isScrolling, true);
    if (success) {
        QString status = !isScrolling ? "开启" : "关闭";
        QMessageBox::information(this, "成功", QString("通知滚动状态已%1！").arg(status));
        refreshNoticeList();
    } else {
        QMessageBox::critical(this, "失败", "通知状态更新失败！");
    }
}

void NoticeManager::on_exportNoticeBtn_clicked()
{
    // 导出所有有效通知
    QList<QVariantMap> notices = DatabaseManager::instance().getValidNotices(false);
    bool success = ExportHelper::exportNoticesToExcel(notices, "所有通知");

    if (!success) {
        QMessageBox::critical(this, "失败", "通知导出失败！\n请确认已安装Excel！");
    }
}

// -------------------------- 辅助函数 --------------------------
void NoticeManager::showAddNoticeDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("添加通知");
    dialog->setModal(true);
    dialog->resize(500, 400);

    // 布局
    QFormLayout* layout = new QFormLayout(dialog);

    // 控件
    QLineEdit* titleEdit = new QLineEdit(dialog);
    titleEdit->setPlaceholderText("请输入通知标题");

    QTextEdit* contentEdit = new QTextEdit(dialog);
    contentEdit->setPlaceholderText("请输入通知内容");
    contentEdit->setMinimumHeight(150);

    QDateTimeEdit* publishTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), dialog);
    publishTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    QDateEdit* expireTimeEdit = new QDateEdit(QDate::currentDate().addDays(7), dialog);
    expireTimeEdit->setDisplayFormat("yyyy-MM-dd");
    expireTimeEdit->setCalendarPopup(true);

    QCheckBox* scrollCheck = new QCheckBox("是否滚动显示", dialog);

    // 按钮布局
    QPushButton* okBtn = new QPushButton("确定", dialog);
    QPushButton* cancelBtn = new QPushButton("取消", dialog);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    // 添加控件到布局
    layout->addRow("标题：", titleEdit);
    layout->addRow("内容：", contentEdit);
    layout->addRow("发布时间：", publishTimeEdit);
    layout->addRow("过期时间：", expireTimeEdit);
    layout->addRow(scrollCheck);
    layout->addRow(btnLayout);

    // 信号槽
    connect(okBtn, &QPushButton::clicked, dialog, [=]() {
        QString title = titleEdit->text().trimmed();
        QString content = contentEdit->toPlainText().trimmed();
        QString publishTime = publishTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString expireTime = expireTimeEdit->date().toString("yyyy-MM-dd");
        bool isScrolling = scrollCheck->isChecked();

        if (title.isEmpty() || content.isEmpty()) {
            QMessageBox::warning(dialog, "警告", "标题和内容不能为空！");
            return;
        }

        // 添加通知
        bool success = DatabaseManager::instance().addNotice(title, content, publishTime, expireTime, isScrolling);
        if (success) {
            QMessageBox::information(dialog, "成功", "通知添加成功！");
            dialog->close();
            refreshNoticeList();
        } else {
            QMessageBox::critical(dialog, "失败", "通知添加失败！");
        }
    });

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::close);

    // 显示对话框
    dialog->exec();
    delete dialog;
}
