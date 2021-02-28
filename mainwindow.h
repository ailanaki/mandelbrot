#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <complex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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

private:
   std::unique_ptr<Ui::MainWindow> ui;

   alignas(32) uchar *buf = nullptr;
   int bytesPerLine = 0;
   QSize buf_size = {320, 320};

   bool is_drag = false;
   QPoint drag_curr_pos = {0, 0};
   QPoint drag_start_pos = {0, 0};

   uint32_t max_iter = 250;
   std::complex<double> upper_left = {-2,1};
   std::complex<double> lower_right = {1,-1};
};
#endif // MAINWINDOW_H
