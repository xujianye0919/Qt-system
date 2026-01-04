#include "SettingsManager.h"
#include <QStandardPaths>
#include <QDir>

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
    // 初始化QSettings（Qt 6，INI格式，存储在AppData）
    QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/classboard_settings.ini";
    m_settings = new QSettings(iniPath, QSettings::IniFormat, this);

    // 加载配置
    m_syncInterval = m_settings->value("Sync/Interval", 600).toInt();
    m_dbPath = m_settings->value("Database/Path", "").toString();
    m_serverUrl = m_settings->value("Server/Url", "http://127.0.0.1:8080/api/sync").toString();
}

// 获取同步间隔
int SettingsManager::getSyncInterval()
{
    return m_syncInterval;
}

// 设置同步间隔
void SettingsManager::setSyncInterval(int secs)
{
    m_syncInterval = secs;
}

// 获取数据库路径
QString SettingsManager::getDbPath()
{
    return m_dbPath;
}

// 设置数据库路径
void SettingsManager::setDbPath(const QString& path)
{
    m_dbPath = path;
}

// 获取服务器地址
QString SettingsManager::getServerUrl()
{
    return m_serverUrl;
}

// 设置服务器地址
void SettingsManager::setServerUrl(const QString& url)
{
    m_serverUrl = url;
}

// 保存所有设置
void SettingsManager::saveSettings()
{
    m_settings->setValue("Sync/Interval", m_syncInterval);
    m_settings->setValue("Database/Path", m_dbPath);
    m_settings->setValue("Server/Url", m_serverUrl);
    m_settings->sync(); // 立即保存
}