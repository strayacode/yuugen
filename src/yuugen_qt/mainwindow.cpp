#include <QtWidgets>
#include "mainwindow.h"
#include <iostream>

// some notes:
// use setCentralWidget for our renderer frontend


MainWindow::MainWindow() {
    CreateMenubar();

    render_timer = new QTimer(this);
    connect(render_timer, SIGNAL(timeout()), this, SLOT(RenderScreen()));

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
    pause_action->setCheckable(true);
    stop_action->setEnabled(false);
    restart_action->setEnabled(false);

    connect(pause_action, &QAction::triggered, this, [this]() {
        if (emu_thread->IsActive()) {
            // first allow the emulator thread to finish a frame and then stop it and the render timer
            emu_thread->Stop();
            render_timer->stop();
        } else {
            emu_thread->Start();
            render_timer->start();
        }
    });

    connect(stop_action, &QAction::triggered, this, [this]() {
        // stop the emulator thread
        emu_thread->Stop();

        pause_action->setEnabled(false);
        stop_action->setEnabled(false);
        restart_action->setEnabled(false);

        // stop the render timer, as there isn't anything to render
        render_timer->stop();
    });

    connect(restart_action, &QAction::triggered, this, [this]() {
        // stop the emulator thread
        emu_thread->Stop();

        // stop the render timer, as there isn't anything to render
        render_timer->stop();

        // do a reset of the core
        core->Reset();
        core->DirectBoot();

        // start the emulator thread again as well as the render timer
        emu_thread->Start();
        render_timer->start();
    });
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // later this will be useful for saving the users configuration
    event->accept();
}

void MainWindow::RenderScreen() {
    // TODO: split the renderwindow into its own separate struct and use setCentralWidget
    update();
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


        // start the draw timer so we can update the screen at 60 fps
        render_timer->start(1000 / 60);
        emu_thread->Start();
    } 
}