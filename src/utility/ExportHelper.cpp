#include "ExportHelper.h"

// 初始化Excel对象
QAxObject* ExportHelper::initExcel()
{
    // 创建Excel应用（Qt 6 QAxObject）
    QAxObject* excel = new QAxObject("Excel.Application");
    if (excel->isNull()) {
        delete excel;
        QMessageBox::critical(nullptr, "错误", "未安装Excel或未注册COM组件！");
        return nullptr;
    }

    // 隐藏Excel窗口
    excel->dynamicCall("SetVisible(bool)", false);
    // 禁用显示警告对话框
    excel->setProperty("DisplayAlerts", false);

    // 创建工作簿
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    workbooks->dynamicCall("Add");
    QAxObject* workbook = excel->querySubObject("ActiveWorkbook");
    QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);

    delete workbooks;
    return worksheet;
}

// 导出课程到Excel
bool ExportHelper::exportCoursesToExcel(const QList<QVariantMap>& courses, const QString& fileName)
{
    if (courses.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "暂无课程数据可导出！");
        return false;
    }

    // 初始化Excel
    QAxObject* worksheet = initExcel();
    if (!worksheet) return false;

    // 表头
    QStringList headers = {"课程ID", "课程名称", "任课教师", "课程类型", "开始时间", "结束时间", "星期", "开始日期", "结束日期", "教室"};
    // 写入表头
    writeToWorksheet(worksheet, courses, headers);

    // 保存文件
    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", fileName, "Excel文件 (*.xlsx)");
    if (savePath.isEmpty()) return false;

    QAxObject* workbook = worksheet->querySubObject("Parent");
    workbook->dynamicCall("SaveAs(const QString&)", savePath);
    workbook->dynamicCall("Close()");

    QAxObject* excel = workbook->querySubObject("Parent");
    excel->dynamicCall("Quit()");

    // 释放资源（Qt 6内存管理）
    delete worksheet;
    delete workbook;
    delete excel;

    QMessageBox::information(nullptr, "成功", QString("课程数据已导出到：%1").arg(savePath));
    return true;
}

// 导出通知到Excel
bool ExportHelper::exportNoticesToExcel(const QList<QVariantMap>& notices, const QString& fileName)
{
    if (notices.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "暂无通知数据可导出！");
        return false;
    }

    QAxObject* worksheet = initExcel();
    if (!worksheet) return false;

    QStringList headers = {"通知ID", "标题", "内容", "发布时间", "过期时间", "是否滚动"};
    writeToWorksheet(worksheet, notices, headers);

    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", fileName, "Excel文件 (*.xlsx)");
    if (savePath.isEmpty()) return false;

    QAxObject* workbook = worksheet->querySubObject("Parent");
    workbook->dynamicCall("SaveAs(const QString&)", savePath);
    workbook->dynamicCall("Close()");

    QAxObject* excel = workbook->querySubObject("Parent");
    excel->dynamicCall("Quit()");

    delete worksheet;
    delete workbook;
    delete excel;

    QMessageBox::information(nullptr, "成功", QString("通知数据已导出到：%1").arg(savePath));
    return true;
}

// 写入Excel工作表
bool ExportHelper::writeToWorksheet(QAxObject* worksheet, const QList<QVariantMap>& data, const QStringList& headers)
{
    if (!worksheet || data.isEmpty() || headers.isEmpty()) return false;

    // 写入表头（第一行）
    for (int i = 0; i < headers.size(); i++) {
        QAxObject* cell = worksheet->querySubObject("Cells(int, int)", 1, i+1);
        cell->dynamicCall("SetValue(const QString&)", headers[i]);
        // 设置表头样式（加粗）
        QAxObject* font = cell->querySubObject("Font");
        font->setProperty("Bold", true);
        delete font;
        delete cell;
    }

    // 写入数据
    for (int row = 0; row < data.size(); row++) {
        QVariantMap rowData = data[row];
        // 课程数据列匹配
        if (headers.contains("课程ID")) {
            QStringList keys = {"id", "course_name", "teacher", "course_type", "start_time", "end_time", "day_of_week", "start_date", "end_date", "classroom"};
            for (int col = 0; col < keys.size(); col++) {
                QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                cell->dynamicCall("SetValue(const QVariant&)", rowData[keys[col]]);
                delete cell;
            }
        }
        // 通知数据列匹配
        else if (headers.contains("通知ID")) {
            QStringList keys = {"id", "title", "content", "publish_time", "expire_time", "is_scrolling"};
            for (int col = 0; col < keys.size(); col++) {
                QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                cell->dynamicCall("SetValue(const QVariant&)", rowData[keys[col]]);
                delete cell;
            }
        }
    }

    // 自动调整列宽
    QAxObject* usedRange = worksheet->querySubObject("UsedRange");
    usedRange->dynamicCall("Columns.AutoFit()");
    delete usedRange;

    return true;
}