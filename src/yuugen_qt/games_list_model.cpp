#include <QByteArray>
#include <fstream>
#include "games_list_model.h"
#include <core/hw/cartridge/nds_loader.h>

GamesListModel::GamesListModel(QObject *parent) : QAbstractTableModel(parent) {
    MapTitlesList("../data/dstdb.txt");
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

    switch (static_cast<ColumnType>(index.column())) {
    case ColumnType::Title:
        return game.title;
    case ColumnType::Size:
        return QLocale().formattedDataSize(game.info.size());
    case ColumnType::Gamecode:
        return game.gamecode_string;
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

        char gamecode_string[5];

        for (int i = 0; i < 4; i++) {
            gamecode_string[i] = (loader.GetGamecode() >> (i * 8)) & 0xFF;
        }

        QString title;

        if (titles_list.count(std::string(gamecode_string))) {
            title = QString::fromStdString(titles_list[std::string(gamecode_string)]);
        } else {
            title = info.fileName();
        }

        gamecode_string[4] = '\0';
        games_list.push_back(Game{path, info, title, QString(gamecode_string)});
    }
}

void GamesListModel::MapTitlesList(QString path) {
    std::ifstream infile(path.toStdString());
    std::string line;
    std::string delimiter = " = ";
    while (std::getline(infile, line)) {
        std::string gamecode = line.substr(0, line.find(delimiter));
        std::string title = line.substr(line.find(delimiter));
        title = title.erase(title.find(delimiter), 3);
        titles_list[gamecode] = title;
    }
}