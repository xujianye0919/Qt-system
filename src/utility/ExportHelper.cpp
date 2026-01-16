#include "ExportHelper.h"

QAxObject* ExportHelper::initExcel(QAxObject*& worksheet, QAxObject*& workbook)
{
    worksheet = nullptr;
    workbook = nullptr;

    QAxObject* excel = new QAxObject("Excel.Application");
    if (excel->isNull()) {
        delete excel;
        return nullptr;
    }

    try {
        excel->dynamicCall("SetVisible(bool)", false);
        excel->setProperty("DisplayAlerts", false);

        QAxObject* workbooks = excel->querySubObject("Workbooks");
        if (workbooks->isNull()) {
            delete workbooks;
            delete excel;
            return nullptr;
        }

        workbooks->dynamicCall("Add");
        delete workbooks;

        workbook = excel->querySubObject("ActiveWorkbook");
        if (workbook->isNull()) {
            delete excel;
            return nullptr;
        }

        worksheet = workbook->querySubObject("Worksheets(int)", 1);
        if (worksheet->isNull()) {
            delete workbook;
            delete excel;
            return nullptr;
        }

        return excel;
    } catch (...) {
        if (worksheet) delete worksheet;
        if (workbook) delete workbook;
        delete excel;
        return nullptr;
    }
}

bool ExportHelper::writeToWorksheet(QAxObject* worksheet, const QList<QVariantMap>& data, const QStringList& headers)
{
    if (!worksheet || data.isEmpty() || headers.isEmpty()) {
        return false;
    }

    try {
        for (int i = 0; i < headers.size(); i++) {
            QAxObject* cell = worksheet->querySubObject("Cells(int, int)", 1, i+1);
            if (cell->isNull()) {
                delete cell;
                return false;
            }

            cell->dynamicCall("SetValue(const QString&)", headers[i]);

            QAxObject* font = cell->querySubObject("Font");
            if (font) {
                font->setProperty("Bold", true);
                font->setProperty("Size", 12);
                font->setProperty("ColorIndex", 2);
                delete font;
            }

            QAxObject* interior = cell->querySubObject("Interior");
            if (interior) {
                interior->setProperty("ColorIndex", 12);
                delete interior;
            }

            delete cell;
        }

        for (int row = 0; row < data.size(); row++) {
            QVariantMap rowData = data[row];

            if (headers.contains("课程ID")) {
                QStringList keys = {"id", "course_name", "teacher", "course_type",
                                   "start_time", "end_time", "day_of_week",
                                   "start_date", "end_date", "classroom_name"};

                for (int col = 0; col < keys.size(); col++) {
                    QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                    if (cell->isNull()) {
                        delete cell;
                        return false;
                    }
                    QVariant cellValue = rowData.value(keys[col], "");
                    if (keys[col] == "day_of_week") {
                        QStringList weekDays = { "", "周一", "周二", "周三", "周四", "周五", "周六", "周日" };
                        int day = cellValue.toInt();
                        cellValue = (day >=1 && day <=7) ? weekDays[day] : "未知";
                    }
                    cell->dynamicCall("SetValue(const QVariant&)", cellValue);
                    delete cell;
                }
            } else if (headers.contains("通知ID")) {
                QStringList keys = {"id", "title", "content", "publish_time", "expire_time", "is_scrolling"};

                for (int col = 0; col < keys.size(); col++) {
                    QAxObject* cell = worksheet->querySubObject("Cells(int, int)", row+2, col+1);
                    if (cell->isNull()) {
                        delete cell;
                        return false;
                    }
                    QVariant cellValue = rowData.value(keys[col], "");
                    if (keys[col] == "is_scrolling") {
                        cellValue = cellValue.toBool() ? "是" : "否";
                    }
                    cell->dynamicCall("SetValue(const QVariant&)", cellValue);
                    delete cell;
                }
            }
        }

        QAxObject* usedRange = worksheet->querySubObject("UsedRange");
        if (usedRange) {
            QAxObject* columns = usedRange->querySubObject("Columns");
            if (columns) {
                columns->dynamicCall("AutoFit()");
                delete columns;
            }
            delete usedRange;
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool ExportHelper::exportCoursesToExcel(const QList<QVariantMap>& courses, const QString& fileName)
{
    if (courses.isEmpty()) {
        return false;
    }

    QAxObject *excel = nullptr, *workbook = nullptr, *worksheet = nullptr;
    excel = initExcel(worksheet, workbook);
    if (!excel || !worksheet || !workbook) {
        if (worksheet) delete worksheet;
        if (workbook) delete workbook;
        if (excel) delete excel;
        return false;
    }

    QStringList headers = {"课程ID", "课程名称", "任课教师", "课程类型", "开始时间", "结束时间", "星期", "开始日期", "结束日期", "教室名称"};
    if (!writeToWorksheet(worksheet, courses, headers)) {
        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");
        delete worksheet;
        delete workbook;
        delete excel;
        return false;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName + ".xlsx";
    defaultPath = defaultPath.replace("/", "\\");
    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", defaultPath, "Excel文件 (*.xlsx)");

    if (savePath.isEmpty()) {
        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");
        delete worksheet;
        delete workbook;
        delete excel;
        return false;
    }

    savePath = savePath.replace("/", "\\");
    try {
        QList<QVariant> saveParams;
        saveParams << savePath << 51 << "" << "" << false << false << 0 << false << false << false << false;
        workbook->dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool, int, bool, bool, bool, bool)", saveParams);

        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");

        delete worksheet;
        delete workbook;
        delete excel;

        return true;
    } catch (...) {
        if (workbook) {
            workbook->dynamicCall("Close()");
            delete workbook;
        }
        if (excel) {
            excel->dynamicCall("Quit()");
            delete excel;
        }
        if (worksheet) delete worksheet;
        return false;
    }
}

bool ExportHelper::exportNoticesToExcel(const QList<QVariantMap>& notices, const QString& fileName)
{
    if (notices.isEmpty()) {
        return false;
    }

    QAxObject *excel = nullptr, *workbook = nullptr, *worksheet = nullptr;
    excel = initExcel(worksheet, workbook);
    if (!excel || !worksheet || !workbook) {
        if (worksheet) delete worksheet;
        if (workbook) delete workbook;
        if (excel) delete excel;
        return false;
    }

    QStringList headers = {"通知ID", "标题", "内容", "发布时间", "过期时间", "是否滚动"};
    if (!writeToWorksheet(worksheet, notices, headers)) {
        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");
        delete worksheet;
        delete workbook;
        delete excel;
        return false;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName + ".xlsx";
    defaultPath = defaultPath.replace("/", "\\");
    QString savePath = QFileDialog::getSaveFileName(nullptr, "导出Excel", defaultPath, "Excel文件 (*.xlsx)");

    if (savePath.isEmpty()) {
        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");
        delete worksheet;
        delete workbook;
        delete excel;
        return false;
    }

    savePath = savePath.replace("/", "\\");
    try {
        QList<QVariant> saveParams;
        saveParams << savePath << 51 << "" << "" << false << false << 0 << false << false << false << false;
        workbook->dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool, int, bool, bool, bool, bool)", saveParams);

        workbook->dynamicCall("Close()");
        excel->dynamicCall("Quit()");

        delete worksheet;
        delete workbook;
        delete excel;

        return true;
    } catch (...) {
        if (workbook) {
            workbook->dynamicCall("Close()");
            delete workbook;
        }
        if (excel) {
            excel->dynamicCall("Quit()");
            delete excel;
        }
        if (worksheet) delete worksheet;
        return false;
    }
}

