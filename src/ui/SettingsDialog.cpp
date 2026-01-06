#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "settings/SettingsManager.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QUrl>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    initUI();
    initConnections();
    loadCurrentSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

// 初始化UI
void SettingsDialog::initUI()
{
    this->setModal(false);  // 改为非模态对话框
    this->setFixedSize(this->size());
    this->setWindowTitle("系统设置");
    this->setAttribute(Qt::WA_DeleteOnClose, true);  // 关闭时自动删除
}

// 初始化信号槽
void SettingsDialog::initConnections()
{
    connect(ui->saveBtn, &QPushButton::clicked, this, &SettingsDialog::handleSaveSettings);
    connect(ui->selectDbBtn, &QPushButton::clicked, this, &SettingsDialog::handleSelectDbPath);
    connect(ui->resetBtn, &QPushButton::clicked, this, &SettingsDialog::handleResetToDefault);
}

// 保存设置
void SettingsDialog::handleSaveSettings()
{
    // 1. 获取界面输入值
    int syncInterval = ui->syncIntervalSpin->value();
    QString dbPath = ui->dbPathEdit->text().trimmed();
    QString serverUrl = ui->serverUrlEdit->text().trimmed();

    // 2. 验证服务器地址格式
    QUrl url(serverUrl);
    if (!url.isValid() && !serverUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "服务器地址格式无效！\n请输入如：http://127.0.0.1:8080/api/sync 的合法URL");
        return;
    }

    // 3. 更新设置管理器
    SettingsManager::instance().setSyncInterval(syncInterval);
    SettingsManager::instance().setDbPath(dbPath);
    SettingsManager::instance().setServerUrl(serverUrl);

    // 4. 保存设置到INI文件
    SettingsManager::instance().saveSettings();

    // 5. 提示并关闭对话框
    QMessageBox::information(this, "成功", "设置已保存！\n部分设置需重启程序生效");
    this->close();  // 直接关闭窗口
}

// 选择数据库路径
void SettingsDialog::handleSelectDbPath()
{
    QString dbPath = QFileDialog::getSaveFileName(
        this,
        "选择/创建数据库文件",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "SQLite数据库 (*.db *.sqlite);;所有文件 (*.*)"
    );

    if (!dbPath.isEmpty()) {
        // 自动补全后缀
        if (!dbPath.endsWith(".db") && !dbPath.endsWith(".sqlite")) {
            dbPath += ".db";
        }
        ui->dbPathEdit->setText(dbPath);

        QMessageBox::information(this, "提示", "数据库路径已选择：\n" + dbPath + "\n重启程序后生效");
    }
}

// 恢复默认设置
void SettingsDialog::handleResetToDefault()
{
    if (QMessageBox::question(this, "确认", "是否恢复所有设置为默认值？\n当前设置将被清空！") != QMessageBox::Yes) {
        return;
    }

    resetToDefault();
    QMessageBox::information(this, "提示", "已恢复默认设置！点击【保存设置】生效");
}

// 加载当前设置
void SettingsDialog::loadCurrentSettings()
{
    ui->syncIntervalSpin->setValue(SettingsManager::instance().getSyncInterval());
    ui->dbPathEdit->setText(SettingsManager::instance().getDbPath());
    ui->serverUrlEdit->setText(SettingsManager::instance().getServerUrl());
    ui->autoSyncCheck->setChecked(true);
}

// 恢复默认设置（界面层）
void SettingsDialog::resetToDefault()
{
    ui->syncIntervalSpin->setValue(600);
    ui->dbPathEdit->setText("");
    ui->serverUrlEdit->setText("http://127.0.0.1:8080/api/sync");
    ui->autoSyncCheck->setChecked(true);
}
