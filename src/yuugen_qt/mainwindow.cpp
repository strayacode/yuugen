#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow() {
    CreateMenubar();

    top_image = QImage(256, 192, QImage::Format_RGB32);
    bottom_image = QImage(256, 192, QImage::Format_RGB32);

    render_timer = new QTimer(this);
    connect(render_timer, SIGNAL(timeout()), this, SLOT(RenderScreen()));

    // account for menubar
    setMinimumSize(256, 384 + 22);

    screen_width = screen_height = 0;
}

void MainWindow::CreateMenubar() {
    CreateFileMenu();
    CreateEmulationMenu();
    CreateViewMenu();
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
    frame_limit_action = emulation_menu->addAction(tr("Frame Limiter"));

    pause_action->setEnabled(false);
    pause_action->setCheckable(true);
    stop_action->setEnabled(false);
    restart_action->setEnabled(false);
    frame_limit_action->setEnabled(false);
    frame_limit_action->setCheckable(true);

    emulation_menu->addSeparator();

    configure_action = emulation_menu->addAction(tr("Configure..."));

    configure_action->setEnabled(true);

    QAction* boot_firmware_action = emulation_menu->addAction(tr("Boot Firmware"));

    connect(boot_firmware_action, &QAction::triggered, this, &MainWindow::BootFirmware);

    connect(pause_action, &QAction::triggered, this, [this]() {
        if (emu_thread->IsActive()) {
            // first allow the emulator thread to finish a frame and then stop it and the render timer
            emu_thread->Stop();
            render_timer->stop();
        } else {
            emu_thread->Start();
            render_timer->start(1000 / 60);
        }
    });

    connect(stop_action, &QAction::triggered, this, [this]() {
        // stop the emulator thread
        emu_thread->Stop();

        pause_action->setEnabled(false);
        stop_action->setEnabled(false);
        restart_action->setEnabled(false);
        frame_limit_action->setEnabled(false);
        configure_action->setEnabled(true);

        // stop the render timer, as there isn't anything to render
        render_timer->stop();
    });

    connect(restart_action, &QAction::triggered, this, [this]() {
        // stop the emulator thread
        emu_thread->Stop();

        // stop the render timer, as there isn't anything to render
        render_timer->stop();

        // do a reset of the core
        core = std::make_unique<Core>();
        emu_thread = std::make_unique<EmuThread>(*core.get(), [this](int fps) {
            UpdateTitle(fps);
        });

        // make sure to set frame limited or not if frame limiter is checked
        if (frame_limit_action->isChecked()) {
            emu_thread->framelimiter = true;
        }

        core->SetRomPath(path.toStdString());
        core->Reset();

        // TODO: change this to check the Config struct first whether we should do direct or firmware boot
        core->DirectBoot();

        // start the emulator thread again as well as the render timer
        emu_thread->Start();
        render_timer->start(1000 / 60);
    });

    connect(frame_limit_action, &QAction::triggered, this, [this]() {
        if (emu_thread) {
            emu_thread->Stop();
            render_timer->stop();

            emu_thread->framelimiter = !emu_thread->framelimiter;

            emu_thread->Start();
            render_timer->start(1000 / 60);
        }
    });

    connect(configure_action, &QAction::triggered, this, &MainWindow::OpenConfigWindow);
}

void MainWindow::OpenConfigWindow() {
    config_window = new ConfigWindow(this);

    config_window->show();
    config_window->raise();
    config_window->activateWindow();
}

void MainWindow::CreateViewMenu() {
    QMenu* view_menu = menuBar()->addMenu(tr("View"));
    QMenu* window_size_menu = view_menu->addMenu(tr("Set Window Size"));

    QAction* window_1x_action = window_size_menu->addAction(tr("Scale 1x"));
    QAction* window_2x_action = window_size_menu->addAction(tr("Scale 2x"));
    QAction* window_4x_action = window_size_menu->addAction(tr("Scale 4x"));

    connect(window_1x_action, &QAction::triggered, this, [this]() {
        resize(256, 384 + 22);  
    });

    connect(window_2x_action, &QAction::triggered, this, [this]() {
        resize(512, 768 + 22);  
    });

    connect(window_4x_action, &QAction::triggered, this, [this]() {
        resize(1024, 1536 + 22);  
    });
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // later this will be useful for saving the users configuration
    event->accept();
}

void MainWindow::RenderScreen() {
    // trigger a paintEvent
    update();
}

void MainWindow::paintEvent(QPaintEvent* event) {
    // only render if the emulator thread is running (slow)
    if (emu_thread) {
        // TODO: split the renderwindow into its own separate struct and use setCentralWidget
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);

        memcpy(top_image.scanLine(0), core->gpu.GetFramebuffer(TOP_SCREEN), 256 * 192 * 4);
        memcpy(bottom_image.scanLine(0), core->gpu.GetFramebuffer(BOTTOM_SCREEN), 256 * 192 * 4);

        QSize window_dimensions = size();

        QImage top_image_scaled = top_image.scaled(window_dimensions.width(), window_dimensions.height() / 2 - 11, Qt::KeepAspectRatio);
        QImage bottom_image_scaled = bottom_image.scaled(window_dimensions.width(), window_dimensions.height() / 2 - 11, Qt::KeepAspectRatio);

        screen_width = top_image_scaled.width();
        screen_height = top_image_scaled.height() * 2;

        painter.drawImage((window_dimensions.width() - top_image_scaled.width()) / 2, 22, top_image_scaled);
        painter.drawImage((window_dimensions.width() - bottom_image_scaled.width()) / 2, bottom_image_scaled.height() + 22, bottom_image_scaled);
    }
}

// TODO: combine into 1 key handler function
void MainWindow::keyPressEvent(QKeyEvent *event) {
    event->accept();

    if (emu_thread) {
        switch (event->key()) {
        case Qt::Key_D:
            core->input.HandleInput(BUTTON_A, true);
            break;
        case Qt::Key_S:
            core->input.HandleInput(BUTTON_B, true);
            break;
        case Qt::Key_Shift:
            core->input.HandleInput(BUTTON_SELECT, true);
            break;
        case Qt::Key_Return:
            core->input.HandleInput(BUTTON_START, true);
            break;
        case Qt::Key_Right:
            core->input.HandleInput(BUTTON_RIGHT, true);
            break;
        case Qt::Key_Left:
            core->input.HandleInput(BUTTON_LEFT, true);
            break;
        case Qt::Key_Up:
            core->input.HandleInput(BUTTON_UP, true);
            break;
        case Qt::Key_Down:
            core->input.HandleInput(BUTTON_DOWN, true);
            break;
        case Qt::Key_E:
            core->input.HandleInput(BUTTON_R, true);
            break;
        case Qt::Key_W:
            core->input.HandleInput(BUTTON_L, true);
            break;
        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    event->accept();

    if (emu_thread) {
        switch (event->key()) {
        case Qt::Key_D:
            core->input.HandleInput(BUTTON_A, false);
            break;
        case Qt::Key_S:
            core->input.HandleInput(BUTTON_B, false);
            break;
        case Qt::Key_Shift:
            core->input.HandleInput(BUTTON_SELECT, false);
            break;
        case Qt::Key_Return:
            core->input.HandleInput(BUTTON_START, false);
            break;
        case Qt::Key_Right:
            core->input.HandleInput(BUTTON_RIGHT, false);
            break;
        case Qt::Key_Left:
            core->input.HandleInput(BUTTON_LEFT, false);
            break;
        case Qt::Key_Up:
            core->input.HandleInput(BUTTON_UP, false);
            break;
        case Qt::Key_Down:
            core->input.HandleInput(BUTTON_DOWN, false);
            break;
        case Qt::Key_E:
            core->input.HandleInput(BUTTON_R, false);
            break;
        case Qt::Key_W:
            core->input.HandleInput(BUTTON_L, false);
            break;
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    event->accept();

    if (emu_thread) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = ((event->y() - 22) / scale) - 192;
        
        if ((y >= 0) && (x >= 0) && event->button() == Qt::LeftButton) {
            core->input.SetTouch(true);
            core->input.SetPoint(x, y);
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();

    if (emu_thread) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = ((event->y() - 22) / scale) - 192;
        
        if ((y >= 0) && (x >= 0) && event->button() == Qt::LeftButton) {
            core->input.SetTouch(false);
            core->input.SetPoint(x, y);
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    event->accept();

    if (emu_thread) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = ((event->y() - 22) / scale) - 192;
        
        if ((y >= 0) && (x >= 0)) {
            core->input.SetPoint(x, y);
        }
    }
}

void MainWindow::LoadRom() {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory("../roms");
    dialog.setNameFilter(tr("NDS ROMs (*.nds)"));

    // pause the emulator thread if one was currently running
    if (emu_thread) {
        emu_thread->Stop();
    }

    // stop the render timer
    render_timer->stop();

    // check if a file was selected
    if (dialog.exec()) {
        // make unique core and emu_thread ptrs
        core = std::make_unique<Core>();
        emu_thread = std::make_unique<EmuThread>(*core.get(), [this](int fps) {
            UpdateTitle(fps);
        });

        // make sure to set frame limited or not if frame limiter is checked
        if (frame_limit_action->isChecked()) {
            emu_thread->framelimiter = true;
        }

        // get the first selection
        path = dialog.selectedFiles().at(0);

        core->SetRomPath(path.toStdString());
        core->Reset();

        // TODO: change this to check the Config struct first whether we should do direct or firmware boot
        core->DirectBoot();

        // allow emulation to be controlled now
        pause_action->setEnabled(true);
        stop_action->setEnabled(true);
        restart_action->setEnabled(true);
        frame_limit_action->setEnabled(true);

        // once the core starts we can't change settings
        configure_action->setEnabled(false);

        // start the draw timer so we can update the screen at 60 fps
        render_timer->start(1000 / 60);
        emu_thread->Start();
    } 
}

void MainWindow::BootFirmware() {
    // pause the emulator thread if one was currently running
    if (emu_thread) {
        emu_thread->Stop();
    }

    // stop the render timer
    render_timer->stop();

    // make unique core and emu_thread ptrs
    core = std::make_unique<Core>();
    emu_thread = std::make_unique<EmuThread>(*core.get(), [this](int fps) {
        UpdateTitle(fps);
    });

    // make sure to set frame limited or not if frame limiter is checked
    if (frame_limit_action->isChecked()) {
        emu_thread->framelimiter = true;
    }

    // give an empty path
    path = "";
    core->SetRomPath(path.toStdString());
    core->Reset();
    core->FirmwareBoot();

    // allow emulation to be controlled now
    pause_action->setEnabled(true);
    stop_action->setEnabled(true);
    restart_action->setEnabled(true);
    frame_limit_action->setEnabled(true);

    // once the core starts we can't change settings
    configure_action->setEnabled(false);

    // start the draw timer so we can update the screen at 60 fps
    render_timer->start(1000 / 60);
    emu_thread->Start();
}

void MainWindow::UpdateTitle(int fps) {
    QString title = "yuugen [" + QString::number(fps) + " FPS | " + QString::number(1000.0 / fps, 'f', 2) + " ms]";
    this->setWindowTitle(title);
}