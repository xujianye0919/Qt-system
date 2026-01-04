#ifndef NOTICEMANAGER_H
#define NOTICEMANAGER_H

#include <QDialog>
#include <QSqlTableModel>
#include "data/DatabaseManager.h"

namespace Ui {
class NoticeManager;
}

class NoticeManager : public QDialog
{
    Q_OBJECT

public:
    explicit NoticeManager(QWidget *parent = nullptr);
    ~NoticeManager();

private slots:
    // 按钮点击事件
    void on_addBtn_clicked();
    void on_deleteBtn_clicked();
    void on_scrollBtn_clicked();
    // 刷新通知列表
    void refreshNoticeList();

private:
    Ui::NoticeManager *ui;
    QSqlTableModel* m_noticeModel; // 通知模型（Qt 6）

    // 弹出添加通知对话框
    void showAddNoticeDialog();
};

#endif // NOTICEMANAGER_H