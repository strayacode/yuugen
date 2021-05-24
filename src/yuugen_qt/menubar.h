#pragma once

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "mainwindow.h"

struct Menubar : public QMenuBar {
Q_OBJECT
public:
    Menubar(QMainWindow* main_window);

private:
    void Exit();

    QMenu* file_menu;
    QAction* exit_action;

    QMainWindow* main_window;
signals:


public slots:

private slots:

};