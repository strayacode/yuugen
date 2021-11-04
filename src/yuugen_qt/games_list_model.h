#pragma once

#include <QAbstractTableModel>
#include <QDirIterator>
#include <QLocale>
#include <vector>
#include <common/types.h>

enum class ColumnType {
    Title = 0,
    Size = 1,
    Gamecode = 2,
};

struct Game {
    QString path;
    QFileInfo info;
    u32 gamecode;
};

class GamesListModel : public QAbstractTableModel {
    Q_OBJECT
public:
    GamesListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;

    std::vector<Game> games_list;
private:
    void AppendGamesList(QString path);

};