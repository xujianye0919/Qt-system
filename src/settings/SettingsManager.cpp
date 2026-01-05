#include "SettingsManager.h"

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
    // 初始化QSettings（INI格式，存储在AppData）
    QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/classboard_settings.ini";
    QDir().mkpath(QFileInfo(iniPath).absolutePath());
    
    m_settings = new QSettings(iniPath, QSettings::IniFormat, this);
    qDebug() << "配置文件路径：" << iniPath;

    // 加载配置
    m_syncInterval = m_settings->value("Sync/Interval", 600).toInt();
    m_dbPath = m_settings->value("Database/Path", "").toString();
    m_serverUrl = m_settings->value("Server/Url", "http://127.0.0.1:8080/api/sync").toString();
    
    qDebug() << "加载配置：同步间隔=" << m_syncInterval 
             << "，数据库路径=" << m_dbPath 
             << "，服务器地址=" << m_serverUrl;
}

// 获取同步间隔
int SettingsManager::getSyncInterval()
{
    return m_syncInterval;
}

// 设置同步间隔
void SettingsManager::setSyncInterval(int secs)
{
    if (secs < 60) secs = 60; // 最小60秒
    if (secs > 3600) secs = 3600; // 最大1小时
    m_syncInterval = secs;
    qDebug() << "设置同步间隔：" << secs;
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
    qDebug() << "设置数据库路径：" << path;
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
    qDebug() << "设置服务器地址：" << url;
}

// 保存所有设置
void SettingsManager::saveSettings()
{
    m_settings->setValue("Sync/Interval", m_syncInterval);
    m_settings->setValue("Database/Path", m_dbPath);
    m_settings->setValue("Server/Url", m_serverUrl);
    m_settings->sync(); // 立即保存
    
    qDebug() << "配置已保存到：" << m_settings->fileName();
}