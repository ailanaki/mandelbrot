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
    QSize sizeHint() const override;


    Q_SIGNALS:

            void blockReady(int cookie);
    void mipmapReady(const QImage&);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent * ev) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void renderMandelbrot();
    void renderMipmap();

    bool isImageReady() const;
    QImage makeImage() const;
    void updateMipmap();

private:
    std::unique_ptr<Ui::MainWindow> ui;

    QImage mipmap;
    unique_ptr_aligned<uchar[]> buf = {nullptr, &free};
    int bytesPerLine = 0;
    int mipmap_size = 320;
    std::complex<double> mipmap_upper_left;
    std::complex<double> mipmap_lower_right;

    QSize buf_size = {320, 320};
    int blockCount = 0;
    int blockCookie = 0;

    bool is_drag = false;
    QPoint drag_start_pos = {0, 0};
    QPoint drag_curr_pos = {0, 0};

    uint32_t max_iter = 150;

    std::complex<double> upper_left = {-2, 1};
    std::complex<double> lower_right = {1, -1};
};
#endif // MAINWINDOW_H
