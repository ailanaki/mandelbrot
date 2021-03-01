#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <complex>
#include <new>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

template<class T>
using unique_ptr_aligned = std::unique_ptr<T, decltype(&free)>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;


    Q_SIGNALS:
            void blockReady();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent * ev) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void renderMandelbrot();

    bool isImageReady() const;
    QImage makeImage() const;

private:
    std::unique_ptr<Ui::MainWindow> ui;

    QImage mipmap;
    unique_ptr_aligned<uchar[]> buf = {nullptr, &free};
    int bytesPerLine = 0;
    QSize buf_size = {320, 320};
    int blockCount = 0;

    bool needUpdateMipmap = false;
    int mipmap_size = 320;
    std::complex<double> mipmap_upper_left;
    std::complex<double> mipmap_lower_right;

    bool is_drag = false;
    QPoint drag_start_pos = {0, 0};

    uint32_t max_iter = 150;
    std::complex<double> upper_left = {-2, 1};
    std::complex<double> lower_right = {1, -1};
};
#endif // MAINWINDOW_H