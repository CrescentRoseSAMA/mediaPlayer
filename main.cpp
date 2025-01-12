#include "videoPlayer.hpp"
#include <QApplication>
#include "media.hpp"
int main(int argc, char *argv[])
{
    qRegisterMetaType<imgInfo>("imgInfo");
    qRegisterMetaType<imgInfo>("imgInfo&");
    PrintInfo("Hello World");
    QApplication app(argc, argv);
    QFile file(":/ui/style/style.qss");
    if (file.open(QFile::ReadOnly))
    {
        QString style = QTextStream(&file).readAll();
        app.setStyleSheet(style);
    }
    mainWindow win;
    win.show();
    return app.exec();
}
