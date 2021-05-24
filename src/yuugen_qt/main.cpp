#include <QApplication>
#include "mainwindow.h"
#include <memory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::unique_ptr<MainWindow> window = std::make_unique<MainWindow>();
    if (!window->Initialise()) {
    	return EXIT_FAILURE;
    }
    
    return a.exec(); 
}
