#pragma once

#include <QDialog>
#include <yuugen_common/emu_thread.h>

struct QTabWidget;
struct QCheckBox;

struct ConfigWindow : public QDialog {
    Q_OBJECT
public:
    explicit ConfigWindow(QWidget* parent, Config& config);

    void SetupLayout();
    void SetupGeneralWidget();
private:
    QTabWidget* tab_widget;
    QWidget* general_widget;

    Config& config;

    QCheckBox* software_fastmem;
    QCheckBox* halt_optimisation;

private slots:
    void OnSaveConfig();
};