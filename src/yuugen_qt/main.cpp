#include <QApplication>
#include "mainwindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Straya");
    QCoreApplication::setApplicationName("yuugen");


    MainWindow window;
    window.show();
    return app.exec(); 
}
