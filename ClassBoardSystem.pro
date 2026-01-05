# Qt 6.9.2 项目配置文件
QT       += core gui widgets sql network concurrent printsupport

# Qt 6 兼容模块
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

# 在ClassBoardSystem.pro中增加：
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 # 禁用Qt 6前的废弃API

# 目标名称与编译模式
TARGET = ClassBoardSystem
TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/data/DatabaseManager.cpp \
    src/network/NetworkWorker.cpp \
    src/ui/MainWindow.cpp \
    src/ui/NoticeManager.cpp \
    src/ui/SettingsDialog.cpp \
    src/settings/SettingsManager.cpp \
    src/utility/TimeHelper.cpp \
    src/utility/ExportHelper.cpp

# 头文件
HEADERS += \
    src/data/DatabaseManager.h \
    src/network/NetworkWorker.h \
    src/ui/MainWindow.h \
    src/ui/NoticeManager.h \
    src/ui/SettingsDialog.h \
    src/settings/SettingsManager.h \
    src/utility/LogHelper.h \
    src/utility/TimeHelper.h \
    src/utility/ExportHelper.h \
    src/utility/src/utility/LogHelper.h

# UI文件
FORMS += \
    src/ui/MainWindow.ui \
    src/ui/NoticeManager.ui \
    src/ui/SettingsDialog.ui

# 资源文件
RESOURCES += \
    src/resource/resource.qrc

# 包含路径
INCLUDEPATH += src

# Release模式优化
CONFIG(release, debug|release): {
    DEFINES += QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS += -O2 -Wall
    QMAKE_LFLAGS += -Wl,-s
    # 输出目录
    DESTDIR = ./release
    OBJECTS_DIR = ./release/obj
    MOC_DIR = ./release/moc
    RCC_DIR = ./release/rcc
    UI_DIR = ./release/ui
}

# Debug模式配置
CONFIG(debug, debug|release): {
    DESTDIR = ./debug
    OBJECTS_DIR = ./debug/obj
    MOC_DIR = ./debug/moc
    RCC_DIR = ./debug/rcc
    UI_DIR = ./debug/ui
}

# Windows平台适配（Excel导出）
win32: {
    QT += axcontainer
    # 拷贝SQL脚本到输出目录
    copy_sql.files = sql/create_tables.sql sql/test_data.sql
    copy_sql.path = $$DESTDIR/sql
    INSTALLS += copy_sql
}
