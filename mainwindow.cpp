#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QImage>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QRunnable>
#include <QThreadPool>
#include <QWheelEvent>

#include <complex>
#include <utility>

int infinity_step(std::complex<double> const &c,int max_iter) {
    std::complex<double> z{0., 0.};
    for (int i = 0; i < max_iter; ++i) {
        z = z * z + c;
        if (std::norm(z) > 4.) {// squared magnitude
            return i;
        }
    }
    return -1;
}

std::complex<double> pix2pt(QSize bounds, QPoint pixel,
                            std::complex<double> upper_left,
                            std::complex<double> lower_right) {
    auto w = lower_right.real() - upper_left.real();
    auto h = upper_left.imag() - lower_right.imag();

    return {upper_left.real() + static_cast<double>(pixel.x()) * w / static_cast<double>(bounds.width()),
            upper_left.imag() - static_cast<double>(pixel.y()) * h / static_cast<double>(bounds.height())};
}

template<typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    connect(this, SIGNAL(blockReady()), SLOT(update()));
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
     if (buf) delete[] buf;
}

QSize MainWindow::minimumSizeHint() const {
    return QSize(1, 1);
}

QSize MainWindow::sizeHint() const{
    return {640, 480};
}

void MainWindow::paintEvent(QPaintEvent * ev){
    if (buf) {
        QPainter painter(this);
        QImage img(buf,
                   buf_size.width(),
                   buf_size.height(),
                   bytesPerLine,
                   QImage::Format_Grayscale8);
        if (is_drag) {
            auto delta = drag_curr_pos - drag_start_pos;
            painter.drawImage(delta.x(), delta.y(), img);
        } else {
            painter.drawImage(0, 0, img);
        }
    }
}


void MainWindow::resizeEvent(QResizeEvent *){
    renderMandelbrot();
}

void MainWindow::wheelEvent(QWheelEvent *event){
    auto center = (lower_right + upper_left) / 2.;
    auto size = lower_right - upper_left;
    auto half = size / 2. + size / 10. * static_cast<double>(sgn(event->delta()));
    auto old_pos = pix2pt(this->size(), event->pos(), upper_left, lower_right);
    auto new_pos = pix2pt(this->size(), event->pos(), center - half, center + half);
    center += old_pos - new_pos;
    upper_left = center - half;
    lower_right = center + half;
    renderMandelbrot();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (is_drag) {
        drag_curr_pos = event->pos();
        repaint();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    is_drag = true;
    drag_start_pos = event->pos();
    drag_curr_pos = drag_start_pos;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    is_drag = false;
    auto size = lower_right - upper_left;
    auto old_pos = pix2pt(this->size(), drag_start_pos, upper_left, lower_right);
    auto new_pos = pix2pt(this->size(), event->pos(), upper_left, lower_right);

    upper_left += old_pos - new_pos;
    lower_right = upper_left + size;

    renderMandelbrot();
}

void MainWindow::renderMandelbrot() {
    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone();

    if (!buf || buf_size != size()) {
        buf_size = size();
        bytesPerLine = static_cast<int>(pow(2., ceil(log2(buf_size.width()))));
        buf = new uchar[bytesPerLine * buf_size.height()];
    }
    memset(buf, 0, bytesPerLine * buf_size.height());
    for (int row = 0; row < buf_size.height(); ++row) {
        auto scanLine = row * bytesPerLine;
        QThreadPool::globalInstance()->start(
                [this, row, scanLine]() {
                    for (int column = 0; column < buf_size.width(); ++column) {
                        auto point = pix2pt(buf_size, {column, row}, upper_left, lower_right);
                        if (auto count = infinity_step(point, max_iter); count != -1) {
                            auto idx = 255. * (count) / max_iter;
                            buf[scanLine + column] = 255 - static_cast<int>(idx) % 256;
                        }
                    }
                    emit blockReady();
                },
                row % 2);
    }
}
