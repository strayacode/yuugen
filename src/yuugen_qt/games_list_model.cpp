#include <QByteArray>
#include "games_list_model.h"
#include <core/hw/cartridge/nds_loader.h>

GamesListModel::GamesListModel(QObject *parent) : QAbstractTableModel(parent) {
    AppendGamesList("../roms/");
}

int GamesListModel::rowCount(const QModelIndex&) const {
   return games_list.size();
}

int GamesListModel::columnCount(const QModelIndex&) const {
    return 3;
}

QVariant GamesListModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Game game = games_list[index.row()];

    char gamecode_string[4];

    for (int i = 0; i < 4; i++) {
        gamecode_string[i] = (game.gamecode >> (i * 8)) & 0xFF;
    }

    switch (static_cast<ColumnType>(index.column())) {
    case ColumnType::Title:
        return game.info.fileName();
    case ColumnType::Size:
        return QLocale().formattedDataSize(game.info.size());
    case ColumnType::Gamecode:
        return QString(gamecode_string);
    }

    return QVariant();
}

QVariant GamesListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return QVariant();
    }
 
    switch (static_cast<ColumnType>(section)) {
    case ColumnType::Title:
        return tr("Title");
    case ColumnType::Size:
        return tr("Size");
    case ColumnType::Gamecode:
        return tr("Gamecode");
    }
    return QVariant();
}

void GamesListModel::AppendGamesList(QString path) {
    const QStringList file_types({"*.nds"});

    QDirIterator it(path, file_types,
        QDir::Files, QDirIterator::Subdirectories
    );

    QStringList list;
    while (it.hasNext()) {
        it.next();

        QString path = it.filePath();
        QFileInfo info(path);

        NDSLoader loader;
        loader.SetPath(path.toStdString());
        loader.LoadHeader();

        games_list.push_back(Game{path, info, loader.GetGamecode()});
    }
}