#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    a.exec();

#ifndef __APPLE__
    exit(0);
#endif
}