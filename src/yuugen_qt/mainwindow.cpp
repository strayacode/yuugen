#include "mainwindow.h"

MainWindow::MainWindow() {

}

bool MainWindow::Initialise() {
    // menubar = new Menubar();
    // setMenuBar(menubar);
	show();

	return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
	close();
}