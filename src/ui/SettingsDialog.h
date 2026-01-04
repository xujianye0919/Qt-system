#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "settings/SettingsManager.h"

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
    void on_saveBtn_clicked();
    void on_selectDbBtn_clicked();

private:
    Ui::SettingsDialog *ui;
    // 加载当前设置到界面
    void loadCurrentSettings();
};

#endif // SETTINGSDIALOG_H