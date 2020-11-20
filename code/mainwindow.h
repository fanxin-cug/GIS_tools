#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QVector>
#include "ogrsf_frmts.h"

#include "delaunay.h"
#define EPSILON 0.000001

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    //自定义排序
    static bool cmp(delaunay_point2d_t a,delaunay_point2d_t b)
    {
        if(a.x<b.x) return true;
        if(abs(a.x-b.x)<EPSILON&&a.y<b.y) return true;
        return false;
    }

private slots:
    void on_actionline_triggered();

    void on_actionclear_triggered();

    void on_actionzoom_in_triggered();

    void on_actionzoom_out_triggered();

    void on_actionup_triggered();

    void on_actionmove_down_triggered();

    void on_actionmove_left_triggered();

    void on_actionmove_right_triggered();

    void on_actionpoint_triggered();

    void on_actionpolygon_triggered();

    void on_actionline_2_triggered();

    void on_actionbuffer_triggered();

    void on_actionrandom_triggered();

private:
    Ui::MainWindow *ui;
    GDALDataset *poDS;
    //int scale;
    int dx;
    int dy;
    QPoint startPos,endPos;
    QVector<QPoint> points;
    QVector<QVector<QPoint>> pntset;
    int state;
    int type;
    QColor *clr;
    double scale;
    //TIN
    struct point2d {
        real	x, y;
    };
    int n;
    delaunay_point2d_t *points_;
    size_t		num_points_;
};

#endif // MAINWINDOW_H
