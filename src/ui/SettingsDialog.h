#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCloseEvent>

// 前置声明
class SettingsManager;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void handleSaveSettings();
    void handleSelectDbPath();
    void handleResetToDefault();

private:
    Ui::SettingsDialog *ui;

    // 辅助函数
    void initUI();
    void initConnections();
    void loadCurrentSettings();
    void resetToDefault();
};

#endif // SETTINGSDIALOG_H
