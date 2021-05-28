#pragma once

#include <QMainWindow>
#include <yuugen_common/emu_thread.h>
#include <memory>
#include <mutex>

struct MainWindow : public QMainWindow {
    Q_OBJECT
public:
	MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();

    QAction* pause_action;
    QAction* stop_action;
    QAction* restart_action;

    std::unique_ptr<Core> core;
    std::unique_ptr<EmuThread> emu_thread;

signals:

public slots:

private slots:
    void LoadRom();

};
