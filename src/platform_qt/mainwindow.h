#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QString>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <nds/nds.h>
// #include <>

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
	QMenu* options_menu;

	QString window_title;

	// actions for the menus
	QAction* load_rom_action;
	QAction* quit_action;
	QAction* set_1x_action;
	QAction* set_2x_action;
	QAction* set_4x_action;

	// strings
	QString rom_path;

	QImage top_screen_image, bottom_screen_image;

	int frames = 0;
	std::chrono::steady_clock::time_point last_frame_time;

	int screen_size_multiplier = 1;

signals:

public slots:

private slots:
	void load_rom();
	void shutdown();
	void set_1x();
	void set_2x();
	void set_4x();
	void set_8x();
};