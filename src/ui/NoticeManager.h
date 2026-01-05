#ifndef NOTICEMANAGER_H
#define NOTICEMANAGER_H

#include <QDialog>
#include <QSqlTableModel>
#include <QStandardItemModel>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDateEdit>
#include <QTimeEdit>
#include <QMessageBox>
#include <QFileDialog>
#include "data/DatabaseManager.h"
#include "utility/ExportHelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NoticeManager; }
QT_END_NAMESPACE

class NoticeManager : public QDialog
{
    Q_OBJECT

public:
    explicit NoticeManager(QWidget *parent = nullptr);
    ~NoticeManager();
    
    // 刷新通知列表
    void refreshNoticeList();

private slots:
    // 按钮点击事件
    void on_addBtn_clicked();
    void on_deleteBtn_clicked();
    void on_scrollBtn_clicked();
    void on_exportNoticeBtn_clicked();

private:
    Ui::NoticeManager *ui;
    QSqlTableModel* m_noticeModel; // 通知模型（Qt 6 SQL模型）
    
    // 辅助函数
    void showAddNoticeDialog();    // 弹出添加通知对话框
    void showEditNoticeDialog();   // 弹出编辑通知对话框（扩展）
};

#endif // NOTICEMANAGER_H