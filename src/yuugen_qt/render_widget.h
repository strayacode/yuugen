#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <common/log.h>

class Core;

class RenderWidget : public QWidget {
    Q_OBJECT
public:
    explicit RenderWidget(QWidget* parent, Core& core);
    void RenderScreen();

private:
    QImage top_image, bottom_image;
    Core& core;

    int screen_width;
    int screen_height;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

};