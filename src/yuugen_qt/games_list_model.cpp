#include <QDirIterator>
#include "games_list_model.h"

GamesListModel::GamesListModel(QObject *parent) : QAbstractTableModel(parent) {
    games_list.append(AppendGamesList("../roms/"));
}

int GamesListModel::rowCount(const QModelIndex & /*parent*/) const {
   return games_list.size();
}

int GamesListModel::columnCount(const QModelIndex & /*parent*/) const {
    return 1;
}

QVariant GamesListModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    QString file = games_list.at(index.row());
    QFileInfo file_info(file);

    switch (static_cast<ColumnType>(index.column())) {
    case ColumnType::Name:
        return file_info.fileName();
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

QStringList GamesListModel::AppendGamesList(QString path) {
    const QStringList file_types({"*.nds"});

    QDirIterator it(path, file_types,
        QDir::Files, QDirIterator::Subdirectories
    );

    QStringList list;
    while (it.hasNext())
    {
        it.next();
        list.append(it.filePath());
    }

    return list;
}