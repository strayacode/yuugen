#include <QPainter>
#include "render_widget.h"
#include <core/core.h>

RenderWidget::RenderWidget(QWidget* parent, Core& core) : QWidget(parent), core(core) {
    top_image = QImage(256, 192, QImage::Format_RGB32);
    bottom_image = QImage(256, 192, QImage::Format_RGB32);

    screen_width = screen_height = 0;
}

void RenderWidget::paintEvent(QPaintEvent* event) {
    // TODO: split the renderwindow into its own separate struct and use setCentralWidget
    if (core.GetState() == State::Running) {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);

        memcpy(top_image.scanLine(0), core.system.gpu.GetFramebuffer(Screen::Top), 256 * 192 * 4);
        memcpy(bottom_image.scanLine(0), core.system.gpu.GetFramebuffer(Screen::Bottom), 256 * 192 * 4);

        QSize window_dimensions = size();

        QImage top_image_scaled = top_image.scaled(window_dimensions.width(), window_dimensions.height() / 2, Qt::KeepAspectRatio);
        QImage bottom_image_scaled = bottom_image.scaled(window_dimensions.width(), window_dimensions.height() / 2, Qt::KeepAspectRatio);

        screen_width = top_image_scaled.width();
        screen_height = top_image_scaled.height() * 2;

        painter.drawImage((window_dimensions.width() - top_image_scaled.width()) / 2, 0, top_image_scaled);
        painter.drawImage((window_dimensions.width() - bottom_image_scaled.width()) / 2, bottom_image_scaled.height(), bottom_image_scaled);
    }
}

void RenderWidget::mousePressEvent(QMouseEvent* event) {
    event->accept();

    if (core.GetState() == State::Running) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = (event->y() / scale) - 192;
        
        if ((y >= 0) && (x >= 0) && event->button() == Qt::LeftButton) {
            core.system.input.SetTouch(true);
            core.system.input.SetPoint(x, y);
        }
    }
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();

    if (core.GetState() == State::Running) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = (event->y() / scale) - 192;
        
        if ((y >= 0) && (x >= 0) && event->button() == Qt::LeftButton) {
            core.system.input.SetTouch(false);
            core.system.input.SetPoint(x, y);
        }
    }
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
    event->accept();

    if (core.GetState() == State::Running) {
        int window_width = size().width();
        float scale = screen_height / 384.0;
        int x = (event->x() - ((window_width - screen_width) / 2)) / scale;
        int y = (event->y() / scale) - 192;
        
        if ((y >= 0) && (x >= 0)) {
            core.system.input.SetPoint(x, y);
        }
    }
}

// TODO: combine into 1 key handler function
void RenderWidget::keyPressEvent(QKeyEvent *event) {
    event->accept();

    if (core.GetState() == State::Running) {
        switch (event->key()) {
        case Qt::Key_D:
            core.system.input.HandleInput(BUTTON_A, true);
            break;
        case Qt::Key_S:
            core.system.input.HandleInput(BUTTON_B, true);
            break;
        case Qt::Key_Shift:
            core.system.input.HandleInput(BUTTON_SELECT, true);
            break;
        case Qt::Key_Return:
            core.system.input.HandleInput(BUTTON_START, true);
            break;
        case Qt::Key_Right:
            core.system.input.HandleInput(BUTTON_RIGHT, true);
            break;
        case Qt::Key_Left:
            core.system.input.HandleInput(BUTTON_LEFT, true);
            break;
        case Qt::Key_Up:
            core.system.input.HandleInput(BUTTON_UP, true);
            break;
        case Qt::Key_Down:
            core.system.input.HandleInput(BUTTON_DOWN, true);
            break;
        case Qt::Key_E:
            core.system.input.HandleInput(BUTTON_R, true);
            break;
        case Qt::Key_W:
            core.system.input.HandleInput(BUTTON_L, true);
            break;
        }
    }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event) {
    event->accept();

    if (core.GetState() == State::Running) {
        switch (event->key()) {
        case Qt::Key_D:
            core.system.input.HandleInput(BUTTON_A, false);
            break;
        case Qt::Key_S:
            core.system.input.HandleInput(BUTTON_B, false);
            break;
        case Qt::Key_Shift:
            core.system.input.HandleInput(BUTTON_SELECT, false);
            break;
        case Qt::Key_Return:
            core.system.input.HandleInput(BUTTON_START, false);
            break;
        case Qt::Key_Right:
            core.system.input.HandleInput(BUTTON_RIGHT, false);
            break;
        case Qt::Key_Left:
            core.system.input.HandleInput(BUTTON_LEFT, false);
            break;
        case Qt::Key_Up:
            core.system.input.HandleInput(BUTTON_UP, false);
            break;
        case Qt::Key_Down:
            core.system.input.HandleInput(BUTTON_DOWN, false);
            break;
        case Qt::Key_E:
            core.system.input.HandleInput(BUTTON_R, false);
            break;
        case Qt::Key_W:
            core.system.input.HandleInput(BUTTON_L, false);
            break;
        }
    }
}

void RenderWidget::RenderScreen() {
    update();
}