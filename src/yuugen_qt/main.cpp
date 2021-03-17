#include <QApplication>
#include <yuugen_qt/mainwindow.h>
#include <memory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    std::unique_ptr<MainWindow> main_window = std::make_unique<MainWindow>();

    return app.exec(); // .exec starts QApplication and related GUI, this line starts 'event loop'

    return 0;
}