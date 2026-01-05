#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QUrl>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // 加载当前设置到界面
    loadCurrentSettings();

    // 设置窗口属性（Qt 6 模态+固定大小）
    this->setModal(true);
    this->setFixedSize(this->size());
    this->setWindowTitle("系统设置 - 教室班牌信息展示系统");

    // 补充：为取消按钮绑定reject()（如果ui里有取消按钮）
    // 如果你的ui里有cancelBtn，添加这行：
    // connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

// -------------------------- 核心槽函数实现 --------------------------
void SettingsDialog::on_saveBtn_clicked()
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

    // 5. 提示并关闭窗口（用accept()而非close()）
    QMessageBox::information(this, "成功", "设置已保存！\n部分设置需重启程序生效");
    this->accept(); // 正确关闭模态对话框
}

void SettingsDialog::on_selectDbBtn_clicked()
{
    // 选择SQLite数据库文件（支持新建/选择已有）
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

        // 提示：切换数据库需重启
        QMessageBox::information(this, "提示", "数据库路径已选择：\n" + dbPath + "\n重启程序后生效");
    }
}

void SettingsDialog::on_resetBtn_clicked()
{
    // 确认恢复默认
    if (QMessageBox::question(this, "确认", "是否恢复所有设置为默认值？\n当前设置将被清空！") != QMessageBox::Yes) {
        return;
    }

    // 恢复界面默认值（不立即保存）
    resetToDefault();
    QMessageBox::information(this, "提示", "已恢复默认设置！点击【保存设置】生效");
}

// -------------------------- 辅助函数实现 --------------------------
void SettingsDialog::loadCurrentSettings()
{
    // 从设置管理器加载当前配置到界面
    ui->syncIntervalSpin->setValue(SettingsManager::instance().getSyncInterval());
    ui->dbPathEdit->setText(SettingsManager::instance().getDbPath());
    ui->serverUrlEdit->setText(SettingsManager::instance().getServerUrl());
    ui->autoSyncCheck->setChecked(true); // 自动同步默认开启
}

void SettingsDialog::resetToDefault()
{
    // 恢复界面默认值（与SettingsManager默认值一致）
    ui->syncIntervalSpin->setValue(600);          // 默认10分钟同步
    ui->dbPathEdit->setText("");                 // 默认使用AppData路径
    ui->serverUrlEdit->setText("http://127.0.0.1:8080/api/sync"); // 默认服务器地址
    ui->autoSyncCheck->setChecked(true);         // 自动同步默认开启
}
