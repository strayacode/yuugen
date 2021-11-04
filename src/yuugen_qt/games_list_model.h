#pragma once

#include <QAbstractTableModel>

enum class ColumnType {
    Name = 0,
};

class GamesListModel : public QAbstractTableModel {
    Q_OBJECT
public:
    GamesListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;

    QStringList games_list;
private:
    QStringList AppendGamesList(QString path);

};