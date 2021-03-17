#include <yuugen_qt/mainwindow.h>

MainWindow::MainWindow() {
    setWindowTitle("yuugen");
    resize(512, 768);
    setMinimumSize(512, 768);


    // tr is used for multiple language support
    file_menu = menuBar()->addMenu(tr("File"));
    emulation_menu = menuBar()->addMenu(tr("Emulation"));

    quit_action = file_menu->addAction(tr("Quit"));
    connect(quit_action, &QAction::triggered, this, &MainWindow::Shutdown);

    show();
}

void MainWindow::Shutdown() {
    printf("shutting down yuugen...\n");
    exit(0);
}