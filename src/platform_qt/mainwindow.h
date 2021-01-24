#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <stdio.h>
#include <stdlib.h>
#include <nds/nds.h>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	bool initialise();
	void closeEvent(QCloseEvent *event);

private:
	NDS nds;

	// menus
	QMenu* file_menu;

	// actions for the menus
	QAction* load_rom_action;

	// strings
	QString rom_path;

signals:

public slots:

private slots:
	void load_rom();
};