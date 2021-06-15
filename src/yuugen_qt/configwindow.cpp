#include "configwindow.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QCheckBox>

ConfigWindow::ConfigWindow(QWidget* parent, Config& config) : QDialog(parent), config(config) {
    SetupLayout();
    SetupGeneralWidget();
}

void ConfigWindow::SetupLayout() {
    // Main Layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Add content to layout before dialog buttons.
    tab_widget = new QTabWidget();
    layout->addWidget(tab_widget);
}

void ConfigWindow::SetupGeneralWidget() {
    general_widget = new QWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(general_widget);

    software_fastmem = new QCheckBox(tr("Enabled Software Fast Memory"));
    halt_optimisation = new QCheckBox(tr("Enable ARM Halt Optimisation"));

    software_fastmem->setChecked(config.software_fastmem);
    halt_optimisation->setChecked(config.halt_optimisation);

    connect(software_fastmem, &QCheckBox::toggled, this, &ConfigWindow::OnSaveConfig);
    connect(halt_optimisation, &QCheckBox::toggled, this, &ConfigWindow::OnSaveConfig);
    layout->addWidget(software_fastmem);
    layout->addWidget(halt_optimisation);

    tab_widget->addTab(general_widget, tr("General"));
}

void ConfigWindow::OnSaveConfig() {
    config.software_fastmem = software_fastmem->isChecked();
    config.halt_optimisation = halt_optimisation->isChecked();
}