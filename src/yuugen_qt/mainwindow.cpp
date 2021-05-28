#include <QtWidgets>
#include "mainwindow.h"

// some notes:
// use setCentralWidget for our renderer frontend


MainWindow::MainWindow() {
    core = std::make_unique<Core>();
    emu_thread = std::make_unique<EmuThread>(*core.get());
    CreateMenubar();

    setMinimumSize(256, 384);
}

void MainWindow::CreateMenubar() {
    CreateFileMenu();
    CreateEmulationMenu();
}

void MainWindow::CreateFileMenu() {
    QMenu* file_menu = menuBar()->addMenu(tr("File"));

    QAction* load_action = file_menu->addAction(tr("Load ROM..."));
    file_menu->addSeparator();
    QAction* exit_action = file_menu->addAction(tr("Exit"));

    connect(exit_action, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::CreateEmulationMenu() {
    QMenu* emulation_menu = menuBar()->addMenu(tr("Emulation"));

    pause_action = emulation_menu->addAction(tr("Pause"));
    stop_action = emulation_menu->addAction(tr("Stop"));
    restart_action = emulation_menu->addAction(tr("Restart"));

    pause_action->setEnabled(false);
    stop_action->setEnabled(false);
    restart_action->setEnabled(false);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // later this will be useful for saving the users configuration
    event->accept();
}