#include <QTableView>
#include <QLabel>
#include "games_list_widget.h"

GamesListWidget::GamesListWidget(QWidget* parent) : QWidget(parent) {
    auto default_label = new QLabel(this);
    default_label->setAlignment(Qt::AlignHCenter);
    default_label->setText(tr("Games List"));
}