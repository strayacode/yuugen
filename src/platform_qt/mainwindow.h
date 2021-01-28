#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds/nds.h>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	bool initialise();
	void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

private:
	NDS nds;

	bool ready = false;

	// menus
	QMenu* file_menu;

	// actions for the menus
	QAction* load_rom_action;
	QAction* quit_action;

	// strings
	QString rom_path;

	QImage top_screen_image, bottom_screen_image;

signals:

public slots:

private slots:
	void load_rom();
	void shutdown();
};