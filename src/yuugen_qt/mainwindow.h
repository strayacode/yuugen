#pragma once

#include <QMainWindow>
#include <core/core.h>
#include <string.h>
#include <memory>
#include <mutex>
#include <functional>

struct MainWindow : public QMainWindow {
    Q_OBJECT
public:
	MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();
    void CreateViewMenu();
    void UpdateTitle(float fps);

    QAction* pause_action;
    QAction* stop_action;
    QAction* restart_action;
    QAction* frame_limit_action;
    QAction* configure_action;

    std::unique_ptr<Core> core;

    QTimer* render_timer;

    QImage top_image, bottom_image;

    int screen_width;
    int screen_height;

    QString path;
signals:

public slots:

private slots:
    void LoadRom();
    void RenderScreen();
    void BootFirmware();
};
