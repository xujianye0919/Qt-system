QT += core gui widgets sql network concurrent printsupport

# 根据Qt版本选择模块
QT_VERSION = $$[QT_VERSION]
QT_MAJOR_VERSION = $$member(QT_VERSION, 0)

# Qt 6.x 版本
greaterThan(QT_MAJOR_VERSION, 5) {
    # 检查core5compat模块是否存在
    !qtHaveModule(core5compat) {
        message("Qt Core5Compat module not found, trying alternative...")
        # 可以使用其他替代方案或编译时忽略
    } else {
        QT += core5compat
    }
}

# Windows平台适配
win32 {
    # 检查axcontainer模块是否存在
    !qtHaveModule(axcontainer) {
        message("ActiveQt (axcontainer) module not found")
        # 如果不需要ActiveX支持，可以注释掉Excel导出功能
        DEFINES += NO_ACTIVEX_SUPPORT
    } else {
        QT += axcontainer
    }
}

# 目标名称与编译模式
TARGET = ClassBoardSystem
TEMPLATE = app