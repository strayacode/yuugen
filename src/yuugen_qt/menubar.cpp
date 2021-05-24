#include "menubar.h"
#include <QMenuBar>

Menubar::Menubar(QMainWindow* main_window) : main_window(main_window) {
    file_menu = addMenu(tr("File"));

    exit_action = file_menu->addAction(tr("Exit"), this, &Menubar::Exit);
}

void Menubar::Exit() {
    close();
}