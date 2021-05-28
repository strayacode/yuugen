#pragma once

#include <QMainWindow>
#include <yuugen_common/emu_thread.h>
#include <string.h>
#include <memory>
#include <mutex>

struct MainWindow : public QMainWindow {
    Q_OBJECT
public:
	MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();
    
    QAction* pause_action;
    QAction* stop_action;
    QAction* restart_action;
    QAction* frame_limit_action;

    std::unique_ptr<Core> core;
    std::unique_ptr<EmuThread> emu_thread;

    QTimer* render_timer;

    QImage top_image, bottom_image;

    QString path;
signals:

public slots:

private slots:
    void LoadRom();
    void RenderScreen();
};
