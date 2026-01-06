#ifndef NOTICEMANAGER_H
#define NOTICEMANAGER_H

#include <QDialog>
#include <QSqlTableModel>
#include <QCloseEvent>

// 前置声明
class DatabaseManager;
class ExportHelper;

namespace Ui {
class NoticeManager;
}

class NoticeManager : public QDialog
{
    Q_OBJECT

public:
    // 普通构造函数（适配成员变量）
    explicit NoticeManager(QWidget *parent = nullptr);
    ~NoticeManager();

    // 刷新通知列表（你的主窗口会调用）
    void refreshNoticeList();

private slots:
    void handleAddNotice();
    void handleDeleteNotice();
    void handleToggleScroll();
    void handleExportNotice();

private:
    Ui::NoticeManager *ui;
    QSqlTableModel *m_noticeModel;

    // 辅助函数
    void initUI();
    void initModel();
    void initConnections();
    void showAddNoticeDialog();
};

#endif // NOTICEMANAGER_H
