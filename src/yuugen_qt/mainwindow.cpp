#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow() :
    core([this](float fps) {
        UpdateTitle(fps);
    }) {

    CreateMenubar();

    render_widget = new RenderWidget(this, core);

    render_timer = new QTimer(this);
    connect(render_timer, &QTimer::timeout, this, &MainWindow::RenderScreen);

    // account for menubar
    setMinimumSize(256, 384 + 22);

    games_list_widget = new GamesListWidget(this);
    
    stack_widget = new QStackedWidget;

    stack_widget->addWidget(games_list_widget);
    stack_widget->addWidget(render_widget);
    setCentralWidget(stack_widget);
}

void MainWindow::RenderScreen() {
    update();

    render_widget->RenderScreen();
}

void MainWindow::CreateMenubar() {
    CreateFileMenu();
    CreateEmulationMenu();
    CreateSettingsMenu();
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

    QAction* boot_firmware_action = emulation_menu->addAction(tr("Boot Firmware"));

    QMenu* boot_mode_menu = emulation_menu->addMenu(tr("Boot Mode"));
    firmware_action = boot_mode_menu->addAction(tr("Firmware"));
    direct_action = boot_mode_menu->addAction(tr("Direct"));

    firmware_action->setCheckable(true);
    direct_action->setCheckable(true);

    firmware_action->setChecked(core.GetBootMode() == BootMode::Firmware);
    direct_action->setChecked(core.GetBootMode() == BootMode::Direct);

    connect(boot_firmware_action, &QAction::triggered, this, &MainWindow::BootFirmware);

    connect(firmware_action, &QAction::triggered, this, [this]() {
        core.SetBootMode(BootMode::Firmware);
        firmware_action->setChecked(core.GetBootMode() == BootMode::Firmware);
        direct_action->setChecked(core.GetBootMode() == BootMode::Direct);
    });

    connect(direct_action, &QAction::triggered, this, [this]() {
        core.SetBootMode(BootMode::Direct);
        firmware_action->setChecked(core.GetBootMode() == BootMode::Firmware);
        direct_action->setChecked(core.GetBootMode() == BootMode::Direct);
    });

    connect(pause_action, &QAction::triggered, this, [this]() {
        if (core.GetState() == State::Running) {
            core.SetState(State::Paused);
            render_timer->stop();
        } else {
            core.SetState(State::Running);
            render_timer->start(1000 / 60);
        }
    });

    connect(stop_action, &QAction::triggered, this, [this]() {
        core.SetState(State::Idle);

        pause_action->setEnabled(false);
        stop_action->setEnabled(false);
        restart_action->setEnabled(false);
        frame_limit_action->setEnabled(false);
        ShowGamesList();

        // stop the render timer, as there isn't anything to render
        render_timer->stop();
    });

    connect(restart_action, &QAction::triggered, this, [this]() {
        core.SetState(State::Idle);

        // stop the render timer, as there isn't anything to render
        render_timer->stop();

        core.SetState(State::Running);

        render_timer->start(1000 / 60);
    });

    connect(frame_limit_action, &QAction::triggered, this, [this]() {
        core.SetState(State::Paused);
        render_timer->stop();

        core.ToggleFramelimiter();

        core.SetState(State::Running);
        render_timer->start(1000 / 60);
    });
}

void MainWindow::CreateSettingsMenu() {
    QMenu* settings_menu = menuBar()->addMenu(tr("Settings"));
    QAction* audio_settings_action = settings_menu->addAction(tr("Audio Settings"));

    connect(audio_settings_action, &QAction::triggered, this, [this]() {
        audio_settings_window = new AudioSettingsWindow(this, core.config);

        audio_settings_window->show();
        audio_settings_window->raise();
        audio_settings_window->activateWindow();
    });
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

void MainWindow::LoadRom() {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory("../roms");
    dialog.setNameFilter(tr("NDS ROMs (*.nds)"));

    core.SetState(State::Idle);
    render_timer->stop();

    if (dialog.exec()) {
        path = dialog.selectedFiles().at(0);

        pause_action->setEnabled(true);
        stop_action->setEnabled(true);
        restart_action->setEnabled(true);
        frame_limit_action->setEnabled(true);
        core.BootGame(path.toStdString());
        ShowScreen();
        render_timer->start(1000 / 60);
    }
}

void MainWindow::BootFirmware() {
    render_timer->stop();

    // allow emulation to be controlled now
    pause_action->setEnabled(true);
    stop_action->setEnabled(true);
    restart_action->setEnabled(true);
    frame_limit_action->setEnabled(true);

    core.BootFirmware();
    ShowScreen();

    // start the draw timer so we can update the screen at 60 fps
    render_timer->start(1000 / 60);
}

void MainWindow::UpdateTitle(float fps) {
    QString title = "yuugen [" + QString::number(fps, 'f', 2) + " FPS | " + QString::number(1000.0 / fps, 'f', 2) + " ms]";
    this->setWindowTitle(title);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    QSize window_dimensions = size();

    games_list_widget->games_list_widget->resize(window_dimensions.width(), window_dimensions.height() - 22);
}

void MainWindow::ShowGamesList() {
    this->setWindowTitle("yuugen");
    stack_widget->setCurrentIndex(0);
}

void MainWindow::ShowScreen() {
    stack_widget->setCurrentIndex(1);
}