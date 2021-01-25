#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QPaintEvent>
#include <stdio.h>
#include <stdlib.h>
#include <nds/nds.h>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	bool initialise();
	void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	NDS nds;

	// menus
	QMenu* file_menu;

	// actions for the menus
	QAction* load_rom_action;
	QAction* quit_action;

	// strings
	QString rom_path;

	QPixmap top_screen_pixmap, bottom_screen_pixmap;

signals:

public slots:

private slots:
	void load_rom();
	void shutdown();
};