#pragma once

#include <QDialog>
#include <QSlider>
#include <core/config.h>

class QCheckBox;

class AudioSettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit AudioSettingsWindow(QWidget* parent, Config& config);

    // void SetupLayout();
    // void SetupGeneralWidget();
private:
    // QTabWidget* tab_widget;
    // QWidget* general_widget;

    Config& config;

    QSlider* volume_slider;

    // QCheckBox* software_fastmem;
    // QCheckBox* halt_optimisation;

private slots:
    // void OnSaveConfig();
};