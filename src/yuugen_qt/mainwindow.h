#pragma once

#include <QMainWindow>
#include <QCloseEvent>

#include <stdio.h>
#include <stdlib.h>
#include <core/core.h>
#include "menubar.h"

struct MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	bool Initialise();
	void closeEvent(QCloseEvent *event);

private:
    Menubar* menubar;

signals:

public slots:

private slots:

};
