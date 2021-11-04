#include "games_list_model.h"

GamesListModel::GamesListModel(QObject *parent) : QAbstractTableModel(parent) {}

int GamesListModel::rowCount(const QModelIndex & /*parent*/) const {
   return 50;
}

int GamesListModel::columnCount(const QModelIndex & /*parent*/) const {
    return 1;
}

QVariant GamesListModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        return QString("hi");
    }

    return QVariant();
}

QVariant GamesListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return QVariant();
    }
 
    switch (static_cast<ColumnType>(section)) {
    case ColumnType::Name:
        return tr("Name");
    }
    return QVariant();
}