#include <QtWidgets>
#include "mainwindow.h"
#include <iostream>

// some notes:
// use setCentralWidget for our renderer frontend


MainWindow::MainWindow() {
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

    connect(load_action, &QAction::triggered, this, &MainWindow::LoadRom);
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

void MainWindow::LoadRom() {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory("../roms");
    dialog.setNameFilter(tr("NDS ROMs (*.nds)"));

    // check if a file was selected
    if (dialog.exec()) {
        // pause the emulator thread if one was currently running
        if (emu_thread) {
            emu_thread->Stop();
        }

        // make unique core and emu_thread ptrs
        core = std::make_unique<Core>();
        emu_thread = std::make_unique<EmuThread>(*core.get());

        // get the first selection
        QString path = dialog.selectedFiles().at(0);

        core->SetRomPath(path.toStdString());
        core->Reset();

        // TODO: change this to check the Config struct first
        core->DirectBoot();

        // allow emulation to be controlled now
        pause_action->setEnabled(true);
        stop_action->setEnabled(true);
        restart_action->setEnabled(true);

        emu_thread->Start();
    } 
}