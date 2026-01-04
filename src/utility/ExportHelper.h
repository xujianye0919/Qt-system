#ifndef EXPORTERHELPER_H
#define EXPORTERHELPER_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include <QAxObject> // Qt 6 axcontainer模块（Windows）
#include <QFileDialog>
#include <QMessageBox>

// 数据导出工具类（Qt 6，仅支持Windows）
class ExportHelper : public QObject
{
    Q_OBJECT
public:
    explicit ExportHelper(QObject *parent = nullptr) : QObject(parent) {}

    // 导出课程数据到Excel
    static bool exportCoursesToExcel(const QList<QVariantMap>& courses, const QString& fileName);

    // 导出通知数据到Excel
    static bool exportNoticesToExcel(const QList<QVariantMap>& notices, const QString& fileName);

private:
    // 初始化Excel对象（Qt 6 QAxObject适配）
    static QAxObject* initExcel();
    // 写入Excel工作表
    static bool writeToWorksheet(QAxObject* worksheet, const QList<QVariantMap>& data, const QStringList& headers);
};

#endif // EXPORTERHELPER_H