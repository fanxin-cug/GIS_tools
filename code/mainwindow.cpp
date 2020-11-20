#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include <QPainter>
#include <QFileDialog>
#include <QMouseEvent>
#include <QInputDialog>

#include<algorithm>
#include<vector>
using namespace std;

#define EPSILON 0.000001

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    poDS(NULL),
    scale(0.5),
    dx(0),
    dy(0),
    num_points_(0)
{
    ui->setupUi(this);
    //CPLSetConfigOption( "GDAL_DATA", "D:/GDAL/data" );//设置环境变量
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");//支持中文路径
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);// 禁止最大化按钮
    setFixedSize(this->width(),this->height());// 禁止拖动窗口大小
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(state==0){//显示shp文件
    GDALAllRegister();
    CPLSetConfigOption("SHAPE_ENCODING","");  //解决中文乱码问题

    if( poDS==NULL)
    {
        painter.end();
        return;
    }

    OGRLayer  *poLayer;
    poLayer = poDS->GetLayer(0); //读取层
    //获取图层范围
    OGREnvelope psExtent;
    poLayer->GetExtent(&psExtent);
    double minX=psExtent.MinX;
    double minY=psExtent.MinY;
    double maxX=psExtent.MaxX;
    double maxY=psExtent.MaxY;

    double dist_x=maxX-minX;
    double dist_y=maxY-minY;

    int width=600;
    int height=600;

    double x_ratio=width/dist_x;
    double y_ratio=height/dist_y;

    double ratio;
    if(x_ratio<y_ratio){
        ratio=x_ratio;
    }else{
        ratio=y_ratio;
    }

    painter.setWindow(QRect(-300, 300, 600, -600));

    OGRFeature *poFeature;
    poLayer->ResetReading();

    while((poFeature = poLayer->GetNextFeature())!=NULL)  //从图层中获取要素
        {
            OGRGeometry *poGeometry=poFeature->GetGeometryRef();   //从要素中获取几何图形
            if (poGeometry!=NULL)
            {
                switch (wkbFlatten(poGeometry->getGeometryType()))
                {
                    case wkbPoint://绘制点shp文件
                    {
                        OGRPoint *poPoint=(OGRPoint*)poGeometry;
                        QPointF p;
                        painter.setPen(QPen(*clr,3));
                        p.setX((width-(maxX-poPoint->getX())*ratio-width/2+dx)*scale);
                        p.setY((height-(maxY-poPoint->getY())*ratio-height/2+dy)*scale);
                        //p.setX((poPoint->getX()-minX)/(maxX-minX)>0.5?(((poPoint->getX()-minX)/(maxX-minX)-0.5)*scale+dx):((0.5-(poPoint->getX()-minX)/(maxX-minX))*(-scale)+dx));
                        //p.setY((poPoint->getY()-minY)/(maxY-minY)>0.5?(((poPoint->getY()-minY)/(maxY-minY)-0.5)*scale+dy):((0.5-(poPoint->getY()-minY)/(maxY-minY))*(-scale)+dy));
                        painter.drawPoint(p);
                        OGRFeature::DestroyFeature( poFeature );
                        break;
                    }
                    case wkbLineString://绘制线shp文件
                    {
                        OGRLineString *poLine = (OGRLineString*)poGeometry;
                        int n=poLine->getNumPoints();
                        QPointF p;
                        QPointF *arr=new QPointF[n];
                        for(int i = 0;i<n;i++)
                        {
                            double staX = poLine->getX(i);
                            double staY = poLine->getY(i);
                            p.setX((width-(maxX-staX)*ratio-width/2+dx)*scale);
                            p.setY((height-(maxY-staY)*ratio-height/2+dy)*scale);
                            //p.setX((staX-minX)/(maxX-minX)>0.5?(((staX-minX)/(maxX-minX)-0.5)*scale+dx):((0.5-(staX-minX)/(maxX-minX))*(-scale)+dx));
                            //p.setY((staY-minY)/(maxY-minY)>0.5?(((staY-minY)/(maxY-minY)-0.5)*scale+dy):((0.5-(staY-minY)/(maxY-minY))*(-scale)+dy));
                            arr[i]=p;
                        }
                        painter.setPen(QPen(*clr));
                        painter.drawPolyline(arr,n);
                        delete[] arr;
                        OGRFeature::DestroyFeature( poFeature );
                        break;
                    }
                    case wkbPolygon://绘制多边形shp文件
                    {
                        OGRPolygon *poPolygon=(OGRPolygon*)poGeometry;
                        OGRLinearRing *poOGRLinearRing = poPolygon->getExteriorRing();
                        int n=poOGRLinearRing->getNumPoints();
                        OGRRawPoint *Gpoints=new OGRRawPoint[n];
                        poOGRLinearRing->getPoints(Gpoints);
                        QPointF p;
                        QPointF *arr=new QPointF[n];
                        for(int i = 0;i<n;i++)
                        {
                            p.setX((width-(maxX-Gpoints[i].x)*ratio-width/2+dx)*scale);
                            p.setY((height-(maxY-Gpoints[i].y)*ratio-height/2+dy)*scale);
                            //p.setX((Gpoints[i].x-minX)/(maxX-minX)>0.5?(((Gpoints[i].x-minX)/(maxX-minX)-0.5)*scale+dx):((0.5-(Gpoints[i].x-minX)/(maxX-minX))*(-scale)+dx));
                            //p.setY((Gpoints[i].y-minY)/(maxY-minY)>0.5?(((Gpoints[i].y-minY)/(maxY-minY)-0.5)*scale+dy):((0.5-(Gpoints[i].y-minY)/(maxY-minY))*(-scale)+dy));
                            arr[i]=p;
                        }
                        painter.setBrush(QBrush(*clr));
                        painter.drawPolygon(arr,n);
                        delete[] arr;
                        OGRFeature::DestroyFeature( poFeature );
                        break;
                    }
                }
            }
        }
    }else if(state==1){//交互式创建shp文件
        if(type==1){//创建点shp文件
            painter.setPen(QPen(QColor(255,0,0),3));
            for(int i=0;i<points.size();i++){
                painter.drawPoint(points[i]);
            }
        }
        if(type==2){//创建线shp文件
            painter.setPen(QPen(QColor(0,255,0),3));
            for(int i=0;i<points.size();i++){
                painter.drawPoint(points[i]);
            }
            painter.setPen(QPen(QColor(0,255,0)));
            for(int i=0;i<pntset.size();i++){
                int n=pntset[i].size();
                QPoint *arr=new QPoint[n];
                for(int j=0;j<n;j++){
                    arr[j]=pntset[i][j];
                }
                painter.drawPolyline(arr,n);
                delete[] arr;
            }
        }
        if(type==3){//创建多边形shp文件
            painter.setPen(QPen(QColor(0,0,255),3));
            for(int i=0;i<points.size();i++){
                painter.drawPoint(points[i]);
            }
            painter.setPen(QPen(QColor(0,0,255)));
            painter.setBrush(QBrush(QColor(0,0,255)));
            for(int i=0;i<pntset.size();i++){
                int n=pntset[i].size();
                QPoint *arr=new QPoint[n];
                for(int j=0;j<n;j++){
                    arr[j]=pntset[i][j];
                }
                painter.drawPolygon(arr,n);
                delete[] arr;
            }
        }
    }else{
        if(num_points_==0){
            painter.end();
            return;
        }
        size_t	i;
        if (num_points_ >= 3)
        {
            delaunay2d_t*	res = delaunay2d_from(&(points_[0]), num_points_);
            tri_delaunay2d_t*	tdel = tri_delaunay2d_from(res);

            for (i = 0; i < tdel->num_triangles; i++)
            {
                QPointF  pnt[3];
                for (int j = 0; j < 3; j++)
                {
                    int p0 = tdel->tris[i * 3 + j];
                    pnt[j] = QPointF(tdel->points[p0].x, tdel->points[p0].y);
                }
                //产生随机颜色
                int a = rand() % 256;
                int b = rand() % 256;
                int c = rand() % 256;
                painter.setBrush(QBrush(QColor(a, b, c)));
                painter.drawPolygon(pnt, 3);
            }

            tri_delaunay2d_release(tdel);

            delaunay2d_release(res);
        }

        for (i = 0; i < num_points_; i++)
        {
            painter.fillRect(QRect(points_[i].x - 2, points_[i].y - 2, 4, 4), QBrush(QColor(0, 0, 0)));
        }

    }
    painter.end();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        if(state==0){//鼠标拖拽平移
            startPos = e->pos();
        }else{//交互式绘制
            startPos = e->pos();
            points.push_back(startPos);
            this->repaint();
        }
    }
    if(e->button() == Qt::RightButton){
        if(type==1){
            QString filename;
            filename = QFileDialog::getSaveFileName(this,tr("Save As"), "", tr("*.shp"));
            if(filename=="") return;
            QByteArray ba = filename.toLocal8Bit();
            const char *pszVectorFile= ba.data();
            int n=points.size();
            QPoint *arr=new QPoint[n];
            for(int i=0;i<n;i++){
                arr[i]=points[i];
            }
            //注册驱动
            const char *pszDriverName="ESRI Shapefile";
            GDALDriver *poDriver;
            GDALAllRegister();
            poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );

            GDALDataset *DS;
            DS = poDriver->Create(pszVectorFile, 0, 0, 0, GDT_Unknown, NULL ); //创建shp文件

            //创建图层
            OGRLayer *poLayer;
            poLayer = DS->CreateLayer( "point", NULL, wkbPoint, NULL );

            OGRFeatureDefn *poDefn=poLayer->GetLayerDefn();

            for(int i=0;i<n;i++){
                OGRFeature* poFeature=OGRFeature::CreateFeature(poDefn);
                OGRPoint *p = (OGRPoint*)OGRGeometryFactory::createGeometry(wkbPoint);
                p->setX(arr[i].x());
                p->setY(-arr[i].y());
                OGRGeometry* pGeo=(OGRGeometry*)p;
                poFeature->SetGeometry(pGeo);

                poLayer->CreateFeature(poFeature);
                OGRFeature::DestroyFeature(poFeature);
            }
            delete[] arr;
            GDALClose( DS );
        }
        if(type==2||type==3){
            pntset.push_back(points);
            points.clear();
            this->repaint();
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e){
    if(e->key()==Qt::Key_Return){
        QString filename;
        filename = QFileDialog::getSaveFileName(this,tr("Save As"), "", tr("*.shp"));
        if(filename=="") return;
        QByteArray ba = filename.toLocal8Bit();
        const char *pszVectorFile= ba.data();
        //注册驱动
        const char *pszDriverName="ESRI Shapefile";
        GDALDriver *poDriver;
        GDALAllRegister();
        poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );

        GDALDataset *DS;
        DS = poDriver->Create(pszVectorFile, 0, 0, 0, GDT_Unknown, NULL ); //创建shp文件

        if(type==2){//线
            OGRLayer *poLayer;
            poLayer = DS->CreateLayer( "line", NULL, wkbLineString, NULL );

            OGRFeatureDefn *poDefn=poLayer->GetLayerDefn();

            for(int i=0;i<pntset.size();i++){
                OGRFeature* poFeature=OGRFeature::CreateFeature(poDefn);
                OGRLineString *p = (OGRLineString*)OGRGeometryFactory::createGeometry(wkbLineString);
                for(int j=0;j<pntset[i].size();j++){
                    p->addPoint(pntset[i][j].x(),-pntset[i][j].y());
                }
                OGRGeometry* pGeo=(OGRGeometry*)p;
                poFeature->SetGeometry(pGeo);

                poLayer->CreateFeature(poFeature);
                OGRFeature::DestroyFeature(poFeature);
            }
        }
        if(type==3){//多边形
            OGRLayer *poLayer;
            poLayer = DS->CreateLayer( "polygon", NULL, wkbPolygon, NULL );

            OGRFeatureDefn *poDefn=poLayer->GetLayerDefn();

            for(int i=0;i<pntset.size();i++){
                OGRFeature* poFeature=OGRFeature::CreateFeature(poDefn);
                OGRPolygon *pGeometry = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
                OGRLinearRing *pRing=(OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
                OGRPoint pt;
                for(int j=0;j<pntset[i].size();j++){
                    pt.setX(pntset[i][j].x());
                    pt.setY(-pntset[i][j].y());
                    pRing->addPoint(&pt);
                }
                pRing->closeRings();
                pGeometry->addRing(pRing);

                OGRGeometry* pGeo=(OGRGeometry*)pGeometry;
                poFeature->SetGeometry(pGeo);

                poLayer->CreateFeature(poFeature);
                OGRFeature::DestroyFeature(poFeature);
            }
        }
        GDALClose( DS );
    }
}

void MainWindow::wheelEvent(QWheelEvent *e)
{
    QPoint sroll = e->angleDelta();
    if(sroll.y()>0){
        scale+=0.1;
        //this->repaint();
    }
    if(sroll.y()<0&&scale>0.2){
        scale-=0.1;
        //this->repaint();
    }
    this->repaint();
}
/*
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    int x,y;
    if (e->buttons() & Qt::LeftButton) {
        endPos=e->pos();
        x=endPos.x()-startPos.x();
        y=startPos.y()-endPos.y();
        dx+=x;
        dy+=y;
        this->repaint();
    }
}
*/
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    int x,y;
    if (e->button()==Qt::LeftButton) {
        endPos=e->pos();
        x=endPos.x()-startPos.x();
        y=startPos.y()-endPos.y();
        dx+=x;
        dy+=y;
        this->repaint();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete[] points_;
}

void MainWindow::on_actionline_triggered()
{
    state=0;//显示
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Vector"), "", "*.shp");
    QByteArray ba = filename.toLocal8Bit();
    const char *p= ba.data();
    poDS = (GDALDataset*) GDALOpenEx(p, GDAL_OF_VECTOR, NULL, NULL, NULL );
    int a = rand() % 256;
    int b = rand() % 256;
    int c = rand() % 256;
    clr=new QColor(a,b,c);
    this->repaint();
}

void MainWindow::on_actionclear_triggered()
{
    if(state==0){//显示
        poDS=NULL;
        scale=0.5;
        dx=0;
        dy=0;
    }
    if(state==1){//绘制
        points.clear();
        pntset.clear();
    }
    if(state==2){//TIN
        num_points_=0;
    }
    this->repaint();
}

void MainWindow::on_actionzoom_in_triggered()
{
    scale+=0.1;
    this->repaint();
}

void MainWindow::on_actionzoom_out_triggered()
{
    if(scale>0.2){
        scale-=0.1;
        this->repaint();
    }
}

void MainWindow::on_actionup_triggered()
{
    dy+=50;
    this->repaint();
}

void MainWindow::on_actionmove_down_triggered()
{
    dy-=50;
    this->repaint();
}

void MainWindow::on_actionmove_left_triggered()
{
    dx-=50;
    this->repaint();
}

void MainWindow::on_actionmove_right_triggered()
{
    dx+=50;
    this->repaint();
}

void MainWindow::on_actionpoint_triggered()
{
    state=1;//编辑状态
    type=1;//点
}

void MainWindow::on_actionpolygon_triggered()
{
    state=1;//编辑状态
    type=3;//多边形
}

void MainWindow::on_actionline_2_triggered()
{
    state=1;//编辑状态
    type=2;//线
}

void MainWindow::on_actionbuffer_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Vector"), "", "*.shp");
    QByteArray ba = filename.toLocal8Bit();
    const char *p= ba.data();

    int r=10;
    int radius = QInputDialog::getInt(NULL, "Input Dialog","Please input the buffer radius", QLineEdit::Normal,r);

    QString filename1 = QFileDialog::getSaveFileName(this,tr("Save As"), "", tr("*.shp"));
    if(filename1=="") return;
    QByteArray ba1 = filename1.toLocal8Bit();
    const char *pszVectorFile= ba1.data();

    GDALAllRegister();
    GDALDataset *DS1 = (GDALDataset*) GDALOpenEx(p, GDAL_OF_VECTOR, NULL, NULL, NULL );
    OGRLayer  *poLayer;
    poLayer = DS1->GetLayer(0); //读取层
    OGRFeature *poFeature;
    //注册驱动
    const char *pszDriverName="ESRI Shapefile";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );

    GDALDataset *DS2 = poDriver->Create(pszVectorFile, 0, 0, 0, GDT_Unknown, NULL ); //创建shp文件

    OGRLayer *newLayer=DS2->CreateLayer("NewLayer");

    while((poFeature = poLayer->GetNextFeature())!=NULL)  //从图层中获取要素
    {
        OGRGeometry *poGeometry=poFeature->GetGeometryRef();   //从要素中获取几何图形
        OGRGeometry *newGeometry=poGeometry->OGRGeometry::Buffer(radius);//生成缓冲区
        OGRFeatureDefn *poDefn=newLayer->GetLayerDefn();
        OGRFeature* newFeature=OGRFeature::CreateFeature(poDefn);
        newFeature->SetGeometry(newGeometry);
        newLayer->CreateFeature(newFeature);
        OGRFeature::DestroyFeature(newFeature);
    }
    GDALClose(DS1);
    GDALClose(DS2);
}

void MainWindow::on_actionrandom_triggered()
{
    state=2;//TIN
    //产生随机点
    n=100;
    int num = QInputDialog::getInt(NULL, "Input Dialog","Please input the number of points", QLineEdit::Normal,n);
    points_=new delaunay_point2d_t[num];
    delaunay_point2d_t	*points=new delaunay_point2d_t[num];
    for (int i=0; i < num; i++){
            points[i].x = rand()%400+100;
            points[i].y = rand()%400+100;
    }
    //排序
    sort(points,points+num,cmp);

    vector<delaunay_point2d_t> v;
    v.push_back(points[0]);
    //去重
    for(int i=1;i<num;i++){
        if(abs(points[i].x-points[i-1].x)<EPSILON&&abs(points[i].y-points[i-1].y)<EPSILON){
            continue;
        }else{
            v.push_back(points[i]);
        }
    }
    delete[] points;
    num_points_=v.size();

    for(int i=0;i<v.size();i++){
        points_[i]=v[i];
    }
    this->repaint();//重绘
}
