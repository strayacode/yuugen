#include <QLabel>
#include <QHeaderView>
#include "games_list_widget.h"
#include "games_list_model.h"

GamesListWidget::GamesListWidget(QWidget* parent) : QWidget(parent) {
    games_list_widget = new QTableView(this);
    auto games_list_model = new GamesListModel(this);

    games_list_widget->setModel(games_list_model);
    games_list_widget->setShowGrid(false);
    games_list_widget->setFrameStyle(QFrame::NoFrame);
    games_list_widget->setAlternatingRowColors(true);
    games_list_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    games_list_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    games_list_widget->verticalHeader()->setDefaultSectionSize(40);
    games_list_widget->setSortingEnabled(true);
    games_list_widget->setCurrentIndex(QModelIndex());
    games_list_widget->setContextMenuPolicy(Qt::CustomContextMenu);
    games_list_widget->setWordWrap(false);

    auto header = games_list_widget->horizontalHeader();
    header->setMinimumSectionSize(40);
    header->setSectionResizeMode(static_cast<int>(ColumnType::Name), QHeaderView::Stretch);

    connect(games_list_widget, &QTableView::doubleClicked, [=](const QModelIndex& index) {
        QString path = games_list_model->games_list.at(index.row());
        emit GameDoubleClicked(path);
    });
}