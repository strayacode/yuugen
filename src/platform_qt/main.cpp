#include <QApplication>
#include "mainwindow.h"
#include <memory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::unique_ptr<MainWindow> window = std::make_unique<MainWindow>();
    if (!window->initialise()) {
    	return EXIT_FAILURE;
    }
    
    // button.show();

    return a.exec(); // .exec starts QApplication and related GUI, this line starts 'event loop'    
}