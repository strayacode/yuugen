#pragma once

#include <QMainWindow>

struct MainWindow : public QMainWindow {
    Q_OBJECT
public:
	MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();

    QAction* pause_action;
    QAction* stop_action;
    QAction* restart_action;

signals:

public slots:

private slots:

};
