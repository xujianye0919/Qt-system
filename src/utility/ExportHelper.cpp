#include "ExportHelper.h"

// 初始化Excel对象
QAxObject* ExportHelper::initExcel()
{
    // 创建Excel应用（Qt 6 QAxObject 适配）
    QAxObject* excel = new QAxObject("Excel.Application");
    if (excel->isNull()) {
        delete excel;
        writeLog("ERROR", "未安装Excel或未注册COM组件", "EXPORT"); // 改用公共日志
        QMessageBox::critical(nullptr, "错误", "未安装Excel或未注册COM组件！\n请安装Microsoft Office后重试。");
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
    writeLog("INFO", "Excel对象初始化成功", "EXPORT"); // 改用公共日志
    return worksheet;
}

// 导出课程到Excel
bool ExportHelper::exportCoursesToExcel(const QList<QVariantMap>& courses, const QString& fileName)
{
    writeLog("INFO", "开始导出课程数据，数量：" + QString::number(courses.size()), "EXPORT"); // 改用公共日志

    if (courses.isEmpty()) {
        writeLog("WARNING", "暂无课程数据可导出", "EXPORT");
        QMessageBox::warning(nullptr, "警告", "暂无课程数据可导出！");
        return false;
    }

    // 初始化Excel
    QAxObject* worksheet = initExcel();
    if (!worksheet) {
        writeLog("ERROR", "Excel初始化失败", "EXPORT");
        return false;
    }

    // 表头
    QStringList headers = {"课程ID", "课程名称", "任课教师", "课程类型", "开始时间", "结束时间", "星期", "开始日期", "结束日期", "教室"};
    // 写入数据
    if (!writeToWorksheet(worksheet, courses, headers)) {
        writeLog("ERROR", "写入Excel失败", "EXPORT");
        return false;
    }

    // 选择保存路径
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName + ".xlsx";
    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", defaultPath, "Excel文件 (*.xlsx)");

    if (savePath.isEmpty()) {
        writeLog("INFO", "用户取消保存", "EXPORT");
        return false;
    }

    // 保存文件（异常捕获）
    try {
        QAxObject* workbook = worksheet->querySubObject("Parent");
        workbook->dynamicCall("SaveAs(const QString&)", savePath);
        workbook->dynamicCall("Close()");

        QAxObject* excel = workbook->querySubObject("Parent");
        excel->dynamicCall("Quit()");

        // 释放资源（Qt 6 内存管理）
        delete worksheet;
        delete workbook;
        delete excel;

        writeLog("INFO", "课程导出成功：" + savePath, "EXPORT");
        QMessageBox::information(nullptr, "成功", QString("课程数据已导出到：\n%1").arg(savePath));
        return true;
    } catch (...) {
        writeLog("ERROR", "Excel保存失败（文件被占用/权限不足）", "EXPORT");
        QMessageBox::critical(nullptr, "失败", "Excel文件保存失败！\n请检查：\n1. 文件是否被其他程序占用\n2. 是否有写入权限");
        return false;
    }
}

// 导出通知到Excel
bool ExportHelper::exportNoticesToExcel(const QList<QVariantMap>& notices, const QString& fileName)
{
    writeLog("INFO", "开始导出通知数据，数量：" + QString::number(notices.size()), "EXPORT");

    if (notices.isEmpty()) {
        writeLog("WARNING", "暂无通知数据可导出", "EXPORT");
        QMessageBox::warning(nullptr, "警告", "暂无通知数据可导出！");
        return false;
    }

    QAxObject* worksheet = initExcel();
    if (!worksheet) return false;

    QStringList headers = {"通知ID", "标题", "内容", "发布时间", "过期时间", "是否滚动"};
    if (!writeToWorksheet(worksheet, notices, headers)) {
        writeLog("ERROR", "写入Excel失败", "EXPORT");
        return false;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName + ".xlsx";
    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", defaultPath, "Excel文件 (*.xlsx)");

    if (savePath.isEmpty()) {
        writeLog("INFO", "用户取消保存", "EXPORT");
        return false;
    }

    try {
        QAxObject* workbook = worksheet->querySubObject("Parent");
        workbook->dynamicCall("SaveAs(const QString&)", savePath);
        workbook->dynamicCall("Close()");

        QAxObject* excel = workbook->querySubObject("Parent");
        excel->dynamicCall("Quit()");

        delete worksheet;
        delete workbook;
        delete excel;

        writeLog("INFO", "通知导出成功：" + savePath, "EXPORT");
        QMessageBox::information(nullptr, "成功", QString("通知数据已导出到：\n%1").arg(savePath));
        return true;
    } catch (...) {
        writeLog("ERROR", "Excel保存失败（文件被占用/权限不足）", "EXPORT");
        QMessageBox::critical(nullptr, "失败", "Excel文件保存失败！\n请检查：\n1. 文件是否被其他程序占用\n2. 是否有写入权限");
        return false;
    }
}

// 写入Excel工作表（核心逻辑）
bool ExportHelper::writeToWorksheet(QAxObject* worksheet, const QList<QVariantMap>& data, const QStringList& headers)
{
    if (!worksheet || data.isEmpty() || headers.isEmpty()) {
        writeLog("ERROR", "写入Excel参数错误（空数据/空表头）", "EXPORT");
        return false;
    }

    try {
        // 写入表头（第一行，加粗+字号12）
        for (int i = 0; i < headers.size(); i++) {
            QAxObject* cell = worksheet->querySubObject("Cells(int, int)", 1, i+1);
            cell->dynamicCall("SetValue(const QString&)", headers[i]);

            // 设置表头样式
            QAxObject* font = cell->querySubObject("Font");
            font->setProperty("Bold", true);
            font->setProperty("Size", 12);
            font->setProperty("ColorIndex", 2); // 白色
            delete font;

            // 表头背景色（深蓝色）
            QAxObject* interior = cell->querySubObject("Interior");
            interior->setProperty("ColorIndex", 12);
            delete interior;

            delete cell;
        }

        // 写入数据行
        for (int row = 0; row < data.size(); row++) {
            QVariantMap rowData = data[row];

            // 课程数据列匹配
            if (headers.contains("课程ID")) {
                QStringList keys = {"id", "course_name", "teacher", "course_type",
                                   "start_time", "end_time", "day_of_week",
                                   "start_date", "end_date", "classroom"};

                for (int col = 0; col < keys.size(); col++) {
                    QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                    QVariant cellValue = rowData.value(keys[col], "");
                    cell->dynamicCall("SetValue(const QVariant&)", cellValue);
                    delete cell;
                }
            }
            // 通知数据列匹配
            else if (headers.contains("通知ID")) {
                QStringList keys = {"id", "title", "content", "publish_time", "expire_time", "is_scrolling"};

                for (int col = 0; col < keys.size(); col++) {
                    QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                    QVariant cellValue = rowData.value(keys[col], "");
                    // 转换布尔值为文字
                    if (keys[col] == "is_scrolling") {
                        cellValue = cellValue.toBool() ? "是" : "否";
                    }
                    cell->dynamicCall("SetValue(const QVariant&)", cellValue);
                    delete cell;
                }
            }
        }

        // 自动调整列宽
        QAxObject* usedRange = worksheet->querySubObject("UsedRange");
        usedRange->dynamicCall("Columns.AutoFit()");
        delete usedRange;

        writeLog("INFO", "Excel数据写入成功，总行数：" + QString::number(data.size() + 1), "EXPORT");
        return true;
    } catch (...) {
        writeLog("ERROR", "Excel数据写入异常（格式错误/数据类型不匹配）", "EXPORT");
        QMessageBox::critical(nullptr, "失败", "Excel数据写入失败！\n请检查数据格式是否合法。");
        return false;
    }
}
