#include <QApplication>
#include "ui/MainWindow.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Qt 6 编码设置（UTF-8）
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    // 设置应用信息
    a.setApplicationName("ClassBoardSystem");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("Qt6Demo");

    // 启动主窗口
    MainWindow w;
    w.show();

    return a.exec();
}