#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <yuugen_common/emuthread.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

struct MainWindow : public QMainWindow {
    MainWindow();

    
    QMenu* file_menu;
    QMenu* emulation_menu;

    QAction* quit_action;

    std::unique_ptr<EmuThread> emu_thread;

signals:

public slots:

private slots:
    void Shutdown();

};