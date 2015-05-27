#include "mainwindow.h"
#include <QApplication>

/* The main program function */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    /* run the main program loop */
    return a.exec();
}
