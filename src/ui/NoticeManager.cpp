#include "NoticeManager.h"
#include "ui_NoticeManager.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>

NoticeManager::NoticeManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NoticeManager)
{
    ui->setupUi(this);

    // 初始化通知模型（Qt 6）
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
    m_noticeModel->select();

    // 绑定到TableView
    ui->noticeTableView->setModel(m_noticeModel);
    ui->noticeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->noticeTableView->verticalHeader()->setVisible(false);

    // 刷新列表
    refreshNoticeList();
}

NoticeManager::~NoticeManager()
{
    delete ui;
}

// 刷新通知列表
void NoticeManager::refreshNoticeList()
{
    m_noticeModel->select(); // 重新查询数据库
}

// 添加通知
void NoticeManager::on_addBtn_clicked()
{
    showAddNoticeDialog();
}

// 删除通知
void NoticeManager::on_deleteBtn_clicked()
{
    // 获取选中行
    QModelIndex index = ui->noticeTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "警告", "请先选中一条通知！");
        return;
    }

    int noticeId = m_noticeModel->data(m_noticeModel->index(index.row(), 0)).toInt();
    if (QMessageBox::question(this, "确认", QString("是否删除ID为%1的通知？").arg(noticeId)) == QMessageBox::Yes) {
        bool success = DatabaseManager::instance().deleteNotice(noticeId);
        if (success) {
            QMessageBox::information(this, "成功", "通知删除成功！");
            refreshNoticeList();
        }
    }
}

// 切换滚动状态
void NoticeManager::on_scrollBtn_clicked()
{
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
        QMessageBox::information(this, "成功", QString("通知滚动状态已切换为：%1").arg(!isScrolling ? "开启" : "关闭"));
        refreshNoticeList();
    }
}

// 弹出添加通知对话框
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
    QTextEdit* contentEdit = new QTextEdit(dialog);
    QLineEdit* publishTimeEdit = new QLineEdit(dialog);
    QLineEdit* expireTimeEdit = new QLineEdit(dialog);
    QCheckBox* scrollCheck = new QCheckBox("是否滚动", dialog);
    QPushButton* okBtn = new QPushButton("确定", dialog);
    QPushButton* cancelBtn = new QPushButton("取消", dialog);

    // 默认值
    publishTimeEdit->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    expireTimeEdit->setPlaceholderText("可选，格式：yyyy-MM-dd");

    // 添加控件到布局
    layout->addRow("标题：", titleEdit);
    layout->addRow("内容：", contentEdit);
    layout->addRow("发布时间：", publishTimeEdit);
    layout->addRow("过期时间：", expireTimeEdit);
    layout->addRow(scrollCheck);

    // 按钮布局
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addRow(btnLayout);

    // 信号槽
    connect(okBtn, &QPushButton::clicked, dialog, [=]() {
        QString title = titleEdit->text().trimmed();
        QString content = contentEdit->toPlainText().trimmed();
        QString publishTime = publishTimeEdit->text().trimmed();
        QString expireTime = expireTimeEdit->text().trimmed();
        bool isScrolling = scrollCheck->isChecked();

        if (title.isEmpty() || content.isEmpty() || publishTime.isEmpty()) {
            QMessageBox::warning(dialog, "警告", "标题、内容、发布时间不能为空！");
            return;
        }

        // 添加通知
        bool success = DatabaseManager::instance().addNotice(title, content, publishTime, expireTime, isScrolling);
        if (success) {
            QMessageBox::information(dialog, "成功", "通知添加成功！");
            dialog->close();
            refreshNoticeList();
        }
    });
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->exec();
    delete dialog;
}