#include "mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QPainter>
#include <QFrame>
#include <QFileDialog>
#include <QImage>

MainWindow::MainWindow() {

}

bool MainWindow::initialise() {
	top_screen_image = QImage(256, 192, QImage::Format_RGB32);
	bottom_screen_image = QImage(256, 192, QImage::Format_RGB32);

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
	if (ready) {
		nds.run_nds_frame();
		frames++;
		std::chrono::duration<double> current_time = std::chrono::steady_clock::now() - last_frame_time;
		if (current_time.count() >= 1.0f)
	    {
	    	window_title = "ChronoDS - " + QString::number(frames) + " FPS";
	    	setWindowTitle(window_title);
	        frames = 0;
	        last_frame_time = std::chrono::steady_clock::now();
    	}
	}

	

	memcpy(top_screen_image.scanLine(0), nds.gpu.get_framebuffer(nds.gpu.TOP_SCREEN), 256*192*4);
    memcpy(bottom_screen_image.scanLine(0), nds.gpu.get_framebuffer(nds.gpu.BOTTOM_SCREEN), 256*192*4);

	painter.drawImage(0, 0, top_screen_image);
	painter.drawImage(0, 192, bottom_screen_image);
	update();
	event->accept();
    
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    event->accept();
    switch (event->key()) {
	case Qt::Key_Z:
        // A
        nds.input.handle_keypress(0, true);
        break;
    case Qt::Key_X:
        // B
        nds.input.handle_keypress(1, true);
        break;
    // should handle X and Y later (not in keyinput)
    case Qt::Key_Shift:
        // select
        nds.input.handle_keypress(2, true);
        break;
    case Qt::Key_Return:
        // start
        nds.input.handle_keypress(3, true);
        break;
    case Qt::Key_Right:
        // right
        nds.input.handle_keypress(4, true);
        break;
    case Qt::Key_Left:
        // left 
        nds.input.handle_keypress(5, true);
        break;
    case Qt::Key_Up:
        // up
        nds.input.handle_keypress(6, true);
        break;
    case Qt::Key_Down:
        // down
        nds.input.handle_keypress(7, true);
        break;
    case Qt::Key_E:
        // Button R
        nds.input.handle_keypress(8, true);
        break;
    case Qt::Key_Q:
        // Button L
        nds.input.handle_keypress(9, true);
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    event->accept();
    switch (event->key()) {
	case Qt::Key_Z:
        // A
        nds.input.handle_keypress(0, false);
        break;
    case Qt::Key_X:
        // B
        nds.input.handle_keypress(1, false);
        break;
    // should handle X and Y later (not in keyinput)
    case Qt::Key_Shift:
        // select
        nds.input.handle_keypress(2, false);
        break;
    case Qt::Key_Return:
        // start
        nds.input.handle_keypress(3, false);
        break;
    case Qt::Key_Right:
        // right
        nds.input.handle_keypress(4, false);
        break;
    case Qt::Key_Left:
        // left 
        nds.input.handle_keypress(5, false);
        break;
    case Qt::Key_Up:
        // up
        nds.input.handle_keypress(6, false);
        break;
    case Qt::Key_Down:
        // down
        nds.input.handle_keypress(7, false);
        break;
    case Qt::Key_E:
        // Button R
        nds.input.handle_keypress(8, false);
        break;
    case Qt::Key_Q:
        // Button L
        nds.input.handle_keypress(9, false);
        break;
    }
}

void MainWindow::load_rom() {
	printf("loading rom...\n");

	rom_path = QFileDialog::getOpenFileName(this, tr("Load NDS ROM"), rom_path, "NDS ROMs (*.nds)");
	nds.direct_boot(rom_path.toStdString());

	ready = true;
}

void MainWindow::shutdown() {
	printf("shutting down the emulator...\n");
	exit(0);
}