#include "mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow() {

}

bool MainWindow::initialise() {
	load_rom_action = new QAction(tr("Load ROM"), this);
	// TODO: add ctrl+O keybind
	connect(load_rom_action, &QAction::triggered, this, &MainWindow::load_rom);

	file_menu = menuBar()->addMenu(tr("File"));
	file_menu->addAction(load_rom_action);

	menuBar()->show();
	setWindowTitle("ChronoDS");
	show();

	// indicates intialisation was successful
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
	printf("exiting...\n");
	exit(1);
}

void MainWindow::load_rom() {
	printf("loading rom...\n");

	rom_path = QFileDialog::getOpenFileName(this, tr("Load NDS ROM"), rom_path, "NDS ROMs (*.nds)");
	nds.direct_boot(rom_path.toStdString());
}