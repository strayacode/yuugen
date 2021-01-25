#include "mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QPainter>
#include <QFrame>
#include <QFileDialog>

MainWindow::MainWindow() {

}

bool MainWindow::initialise() {
	load_rom_action = new QAction("Load ROM...", this);
	quit_action = new QAction("Quit", this);
	// TODO: add ctrl+O keybind
	connect(load_rom_action, &QAction::triggered, this, &MainWindow::load_rom);
	connect(quit_action, &QAction::triggered, this, &MainWindow::shutdown);

	file_menu = menuBar()->addMenu("File");
	file_menu->addAction(load_rom_action);
	file_menu->addAction(quit_action);

	menuBar()->show();
	setWindowTitle("ChronoDS");
	resize(256, 384);
	setMinimumSize(256, 384);
	show();

	// indicates intialisation was successful
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
	printf("exiting...\n");
	exit(1);
}

void MainWindow::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.fillRect(rect(), Qt::black);

	painter.drawPixmap(0, 0, top_screen_pixmap);
	painter.drawPixmap(0, 192, bottom_screen_pixmap);
	event->accept();
    
}

void MainWindow::load_rom() {
	printf("loading rom...\n");

	rom_path = QFileDialog::getOpenFileName(this, tr("Load NDS ROM"), rom_path, "NDS ROMs (*.nds)");
	nds.direct_boot(rom_path.toStdString());
}

void MainWindow::shutdown() {
	printf("shutting down the emulator...\n");
	exit(0);
}