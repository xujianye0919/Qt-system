#ifndef EXPORTERHELPER_H
#define EXPORTERHELPER_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include <QAxObject>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QThread>

class ExportHelper : public QObject
{
    Q_OBJECT
public:
    explicit ExportHelper(QObject *parent = nullptr) : QObject(parent) {}

    static bool exportCoursesToExcel(const QList<QVariantMap>& courses, const QString& fileName);
    static bool exportNoticesToExcel(const QList<QVariantMap>& notices, const QString& fileName);

private:
    static QAxObject* initExcel(QAxObject*& worksheet, QAxObject*& workbook);
    static bool writeToWorksheet(QAxObject* worksheet, const QList<QVariantMap>& data, const QStringList& headers);
};

#endif // EXPORTERHELPER_H
