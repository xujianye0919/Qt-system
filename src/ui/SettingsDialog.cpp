#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "data/DatabaseManager.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    // 加载当前设置
    loadCurrentSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

// 加载当前设置
void SettingsDialog::loadCurrentSettings()
{
    ui->syncIntervalSpin->setValue(SettingsManager::instance().getSyncInterval());
    ui->dbPathEdit->setText(SettingsManager::instance().getDbPath());
    ui->serverUrlEdit->setText(SettingsManager::instance().getServerUrl());
}

// 保存设置
void SettingsDialog::on_saveBtn_clicked()
{
    // 保存到设置管理器
    SettingsManager::instance().setSyncInterval(ui->syncIntervalSpin->value());
    SettingsManager::instance().setDbPath(ui->dbPathEdit->text());
    SettingsManager::instance().setServerUrl(ui->serverUrlEdit->text());
    SettingsManager::instance().saveSettings();

    // 如果数据库路径变更，重新初始化数据库
    if (!ui->dbPathEdit->text().isEmpty()) {
        DatabaseManager::instance().init(ui->dbPathEdit->text());
    }

    QMessageBox::information(this, "成功", "设置保存成功！");
    this->close();
}

// 选择数据库路径
void SettingsDialog::on_selectDbBtn_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "选择数据库文件", "", "SQLite数据库 (*.db *.sqlite)");
    if (!filePath.isEmpty()) {
        ui->dbPathEdit->setText(filePath);
    }
}