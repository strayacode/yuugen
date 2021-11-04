#pragma once

#include <QWidget>
#include <QTableView>

class GamesListWidget : public QWidget {
    Q_OBJECT
public:
    GamesListWidget(QWidget* parent);

    QTableView* games_list_widget;
private:

signals:
    void GameDoubleClicked(QString path);

};