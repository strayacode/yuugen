#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <core/core.h>
#include <string.h>
#include <memory>
#include <mutex>
#include <functional>
#include "audio_settings_window.h"
#include "games_list_widget.h"
#include "render_widget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
	MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();
    void CreateSettingsMenu();
    void CreateViewMenu();
    void UpdateTitle(float fps);
    void ShowGamesList();
    void ShowScreen();

    QAction* pause_action;
    QAction* stop_action;
    QAction* restart_action;
    QAction* frame_limit_action;
    QAction* firmware_action;
    QAction* direct_action;
    QStackedWidget* stack_widget;
    GamesListWidget* games_list_widget;
    RenderWidget* render_widget;

    Core core;

    QTimer* render_timer;

    QString path;

    AudioSettingsWindow* audio_settings_window = nullptr;
signals:

public slots:

private slots:
    void LoadRom();
    void RenderScreen();
    void BootFirmware();
};
