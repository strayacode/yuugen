#include "mainwindow.h"

MainWindow::MainWindow() {

}

bool MainWindow::Initialise() {
    menubar = new Menubar(QMainWindow(this));
    setMenuBar(menubar);
	show();

	return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
	close();
}