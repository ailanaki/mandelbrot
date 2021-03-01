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
#include <cstdlib>
#include <cstring>

using namespace std;

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

QPoint pt2pix(QRect bounds,
              std::complex<double> point,
              std::complex<double> upper_left,
              std::complex<double> lower_right) {
    auto w = lower_right.real() - upper_left.real();
    auto h = upper_left.imag() - lower_right.imag();
    auto p = point - upper_left;
    return {static_cast<int>(bounds.x() + p.real() * static_cast<double>(bounds.width()) / w),
            static_cast<int>(bounds.y() - p.imag() * static_cast<double>(bounds.height()) / h)};
}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    connect(
            this, &MainWindow::blockReady, this, [this]() {
                if (++blockCount == buf_size.height())
                    repaint();
            },
            Qt::QueuedConnection);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{}

QSize MainWindow::minimumSizeHint() const {
    return QSize(1, 1);
}

QSize MainWindow::sizeHint() const{
    return {640, 480};
}

void MainWindow::paintEvent(QPaintEvent *){
    QPainter painter(this);
    if (is_drag || !isImageReady()) {
        auto ul = pt2pix(rect(), mipmap_upper_left, upper_left, lower_right);
        auto lr = pt2pix(rect(), mipmap_lower_right, upper_left, lower_right);
        painter.drawImage(QRect{ul, lr}, mipmap);
    } else if (isImageReady()) {
        painter.drawImage(0, 0, makeImage());
        if (needUpdateMipmap) {
            needUpdateMipmap = false;
            mipmap_upper_left = upper_left;
            mipmap_lower_right = lower_right;
            mipmap = makeImage().scaled(mipmap_size, mipmap_size, Qt::KeepAspectRatio);
        }
    }
}


void MainWindow::resizeEvent(QResizeEvent *){
    renderMandelbrot();
}

void MainWindow::wheelEvent(QWheelEvent *event){
    auto center = (lower_right + upper_left) / 2.;
    auto size = lower_right - upper_left;
    auto half = size / 2. + size / 10. * (event->delta() < 0 ? -1. : 1.);
    auto old_pos = pix2pt(this->size(), event->pos(), upper_left, lower_right);
    auto new_pos = pix2pt(this->size(), event->pos(), center - half, center + half);
    center += old_pos - new_pos;
    upper_left = center - half;
    lower_right = center + half;
    renderMandelbrot();
    update();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (is_drag) {
        auto size = lower_right - upper_left;
        auto old_pos = pix2pt(this->size(), drag_start_pos, upper_left, lower_right);
        auto new_pos = pix2pt(this->size(), event->pos(), upper_left, lower_right);

        drag_start_pos = event->pos();
        upper_left += old_pos - new_pos;
        lower_right = upper_left + size;
        repaint();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    is_drag = true;
    drag_start_pos = event->pos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *){
    is_drag = false;
    renderMandelbrot();
}

void MainWindow::renderMandelbrot() {
    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone();

    if (!buf || buf_size != size()) {
        buf_size = size();
        bytesPerLine = static_cast<int>(pow(2., ceil(log2(buf_size.width()))));
        buf = {static_cast<uchar *>(aligned_alloc(32, bytesPerLine * buf_size.height())), &free};
    }
    memset(buf.get(), 0, bytesPerLine * buf_size.height());
    blockCount = 0;
    needUpdateMipmap = true;
    for (int row = 0; row < buf_size.height(); ++row) {
        auto scanLine = row * bytesPerLine;
        QThreadPool::globalInstance()->start(
                [this, row, scanLine]() {
                    for (int column = 0; column < buf_size.width(); ++column) {
                        auto point = pix2pt(buf_size, {column, row}, upper_left, lower_right);
                        if (auto count = infinity_step(point, max_iter); count != -1) {
                            auto idx = 255. * (count) / max_iter;
                            (buf.get())[scanLine + column] = 255 - static_cast<int>(idx) % 256;
                        }
                    }
                    emit blockReady();
                });
    }
}
QImage MainWindow::makeImage() const {
    return QImage(buf.get(),
                  buf_size.width(),
                  buf_size.height(),
                  bytesPerLine,
                  QImage::Format_Grayscale8);
}
bool MainWindow::isImageReady() const {
    return buf && blockCount == buf_size.height();
}