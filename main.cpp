#include "MainWindow.h"
#include <QApplication>
#include <QDebug> // 添加头文件

int main(int argc, char *argv[])
{
    qDebug() << "进入main函数"; // 调试输出
    QApplication a(argc, argv);
    qDebug() << "创建MainWindow实例"; // 调试输出
    MainWindow w;
    qDebug() << "调用w.show()"; // 调试输出
    w.show();
    qDebug() << "进入事件循环"; // 调试输出
    return a.exec();
}
