#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include "settings/SettingsManager.h"
#include "data/DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void on_saveBtn_clicked();    // 保存设置
    void on_selectDbBtn_clicked();// 选择数据库路径
    void on_resetBtn_clicked();   // 恢复默认设置

private:
    Ui::SettingsDialog *ui;
    
    // 加载当前设置到界面
    void loadCurrentSettings();
    // 恢复默认设置（不保存）
    void resetToDefault();
};

#endif // SETTINGSDIALOG_H