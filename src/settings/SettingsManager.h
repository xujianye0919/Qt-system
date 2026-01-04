#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

// 设置管理类（单例，Qt 6 QSettings适配）
class SettingsManager : public QObject
{
    Q_OBJECT
public:
    static SettingsManager& instance();
    ~SettingsManager() = default;

    // 获取/设置同步间隔（秒）
    int getSyncInterval();
    void setSyncInterval(int secs);

    // 获取/设置数据库路径
    QString getDbPath();
    void setDbPath(const QString& path);

    // 获取/设置服务器地址
    QString getServerUrl();
    void setServerUrl(const QString& url);

    // 保存所有设置
    void saveSettings();

private:
    SettingsManager(QObject *parent = nullptr);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings* m_settings; // Qt 6配置管理器（INI格式）
    // 默认配置
    int m_syncInterval = 600;
    QString m_dbPath = "";
    QString m_serverUrl = "http://127.0.0.1:8080/api/sync";
};

#endif // SETTINGSMANAGER_H