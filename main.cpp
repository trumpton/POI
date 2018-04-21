#include "mainwindow.h"
#include <QApplication>

//
// Notes:
//
// To debug javascript, launch with command line option of:
//
//  --remote-debugging-port=23456
//

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}
