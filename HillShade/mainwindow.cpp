#include <iostream>
#include <iomanip>
#include <QFileDialog>
#include <fstream>
#include <string>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#define pi 3.14
#define cellsize 90
#define z_factor 10
using namespace std;
typedef  unsigned char  BYTE;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("HillShading v2.0");
    initUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUi()
{
    ui->label_1->setText("");
    ui->label_1->setStyleSheet("QLabel{background-color:rgb(200,200,200);}");
    ui->label_4->setText("");
    ui->label_4->setStyleSheet("QLabel{background-color:rgb(200,200,200);}");
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    ui->lineEdit_2->setText("45");
    ui->lineEdit_3->setText("315");
    connect(ui->pushButton_load, SIGNAL(clicked()), this, SLOT(on_pushButton_load_clicked()),Qt::UniqueConnection);
    connect(ui->pushButton_begin, SIGNAL(clicked()), this, SLOT(on_pushButton_begin_clicked()),Qt::UniqueConnection);
    connect(ui->pushButton_close, SIGNAL(clicked()), SLOT(close()));
}


void MainWindow::on_radioButton_clicked(bool checked)
{
    if(checked==true)alpha=1;
    InitColorTable_grey();
    tableRGB_output();
    DisplayRGBTable(tableRGB_Path);

}


void MainWindow::on_radioButton_2_clicked(bool checked)
{
    if(checked==true)alpha=0.4;
    QString Color_fileName = QFileDialog::getOpenFileName(this,tr("打开颜色表文件"),"E:/",tr("color files(*.color);"));
    InitColorTable_color(Color_fileName);
    tableRGB_output();
    DisplayRGBTable(tableRGB_Path);

}


void MainWindow::on_pushButton_load_clicked()
{
    ui->label_1->clear();
    Tiff_fileName = QFileDialog::getOpenFileName(this,tr("打开TIFF文件"),"E:/",tr("images(*.tif);"));
    ui->lineEdit->setText(Tiff_fileName);
    QString path = ui->lineEdit->text();
    QByteArray buffer = path.toLatin1();
    Tif_Path = buffer.data();
    GDALAllRegister();
    Tif_Data = (GDALDataset*)GDALOpen(Tif_Path, GA_ReadOnly);
    if (Tif_Data == nullptr)
    {
        ui->textBrowser_Feedback->append("无法载入Tif图像，请检查Tif文件路径与命名！");
    }
    else
    {
        ui->textBrowser_Feedback->append("已打开位于"+ui->lineEdit->text()+"的Tif文件");
        //DisplayTifImage(Tif_Path);
        DisplayTifInfo(Tif_Data);
    }
}


void MainWindow::reuse()
{
    QString path = ui->lineEdit->text();
    QByteArray buffer = path.toLatin1();
    Tif_Path = buffer.data();
    GDALAllRegister();
    Tif_Data = (GDALDataset*)GDALOpen(Tif_Path, GA_ReadOnly);
}


void MainWindow::DisplayTifImage(const char* Path)
{
    QPixmap tif_Image(Path);
    tif_Image = tif_Image.scaled(ui->label_1->size(), Qt::KeepAspectRatio);
    ui->label_1->setPixmap(tif_Image);
    ui->label_1->setAlignment(Qt::AlignCenter);
    ui->label_1->show();
}

void MainWindow::DisplayRGBTable(const char* Path)
{
    QPixmap RGBTable_Image(Path);
    RGBTable_Image =RGBTable_Image.scaled(ui->label_4->size(), Qt::KeepAspectRatio);
    ui->label_4->setPixmap(RGBTable_Image);
    ui->label_4->setAlignment(Qt::AlignCenter);
    ui->label_4->show();
}


void MainWindow::DisplayTifInfo(GDALDataset* Tif_Data)
{
    //输出待处理Tif文件的地理参考信息。
    ui->textBrowser_Info->append("Driver: ");
    ui->textBrowser_Info->insertPlainText(Tif_Data->GetDriver()->GetDescription());
    ui->textBrowser_Info->insertPlainText("/");
    ui->textBrowser_Info->insertPlainText(Tif_Data->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
    ui->textBrowser_Info->append("Tif数据分辨率：");
    ui->textBrowser_Info->insertPlainText(QString::number(Tif_Data->GetRasterXSize()));
    ui->textBrowser_Info->insertPlainText("*");
    ui->textBrowser_Info->insertPlainText(QString::number(Tif_Data->GetRasterYSize()));
    ui->textBrowser_Info->insertPlainText("*");
    ui->textBrowser_Info->insertPlainText(QString::number(Tif_Data->GetRasterCount()));
    if (Tif_Data->GetProjectionRef() != NULL)
    ui->textBrowser_Info->append("Tif数据地图投影：");
    ui->textBrowser_Info->insertPlainText(Tif_Data->GetProjectionRef());
    double adfGeoTransform[6];
    if (Tif_Data->GetGeoTransform(adfGeoTransform) == CE_None)
    ui->textBrowser_Info->append("Origin: (");
    ui->textBrowser_Info->insertPlainText(QString::number(adfGeoTransform[0]));
    ui->textBrowser_Info->insertPlainText(",");
    ui->textBrowser_Info->insertPlainText(QString::number(adfGeoTransform[3]));
    ui->textBrowser_Info->insertPlainText(")");
    ui->textBrowser_Info->append("Pixel Size: ");
    ui->textBrowser_Info->insertPlainText(QString::number(adfGeoTransform[1]));
    ui->textBrowser_Info->insertPlainText(",");
    ui->textBrowser_Info->insertPlainText(QString::number(adfGeoTransform[5]));
    ui->textBrowser_Info->insertPlainText(")");
    ui->textBrowser_Info->append("-------------------------------------------");
}


void MainWindow::on_pushButton_begin_clicked()
{
    ui->progressBar->setValue(0);
    ui->label_1->clear();
    MainWindow::reuse();
    float Altitude = ui->lineEdit_2->text().toFloat();
    float Zimuth = ui->lineEdit_3->text().toFloat();

    float Zenith_deg = 90 - Altitude;
    float Zenith_rad = Zenith_deg * (pi / 180);

    float Azimuth_math = 360.0 - Zimuth + 90;
    if (Azimuth_math > 360)Azimuth_math = Azimuth_math - 360;
    float Azimuth_rad = Azimuth_math * (pi / 180);

    if(Tif_Data==nullptr)
    {
        ui->textBrowser_Feedback->append("请先载入待处理TIFF图像。");
    }
    else if((ui->lineEdit_2->text()=="")||(ui->lineEdit_3->text()==""))
    {
        ui->textBrowser_Feedback->append("请输入太阳高度角与太阳方位角信息。");
    }
    else if((Altitude>90||Altitude<0)||(Zimuth>360||Zimuth<0))
    {
        ui->textBrowser_Feedback->append("太阳高度角与太阳方位角大小超出范围。");
    }
    else if((ui->radioButton->isChecked()==false)&&(ui->radioButton_2->isChecked()==false))
    {
        ui->textBrowser_Feedback->append("请选择单色晕渲或彩色晕渲。");
    }
    else
    {
        QString Tiff_fileName_new;
        QString Tiff_fileName_tmp = Tiff_fileName;
        QString HillShade_kind;
        Tiff_fileName_tmp.chop(4);
        if(alpha==1)
        {
            HillShade_kind="_grey.tif";
            Tiff_fileName_new=Tiff_fileName_tmp+HillShade_kind;
        }
        else if(alpha==0.4)
        {
            HillShade_kind="_color.tif";
            Tiff_fileName_new=Tiff_fileName_tmp+HillShade_kind;
        }
        QString Hillshade_Path_string=Tiff_fileName_new;
        QByteArray buffer = Hillshade_Path_string.toLatin1();
        Hillshade_Path = buffer.data();
        Image_output(Zenith_rad,Azimuth_rad,alpha);
        DisplayTifImage(Hillshade_Path);
    }
}


struct Tif_RGB
{
    float R;
    float G;
    float B;

    Tif_RGB()
    {

    }

    Tif_RGB(float r, float g, float b)
    {
        R = r;
        G = g;
        B = b;
    }

    Tif_RGB(const Tif_RGB& rgb)
    {
        R = rgb.R;
        G = rgb.G;
        B = rgb.B;
    }

    Tif_RGB& operator= (const Tif_RGB& rgb)
    {
        R = rgb.R;
        G = rgb.G;
        B = rgb.B;
        return *this;
    }
};


void Gradient(Tif_RGB &start, Tif_RGB &end, vector<Tif_RGB> &RGBList)
{
    float dr = (end.R - start.R) / RGBList.size();
    float dg = (end.G - start.G) / RGBList.size();
    float db = (end.B - start.B) / RGBList.size();
    for (unsigned int i = 0; i < RGBList.size(); i++)
    {
        RGBList[i].R = start.R + dr * i;
        RGBList[i].G = start.G + dg * i;
        RGBList[i].B = start.B + db * i;
    }
}


vector<Tif_RGB> tableRGB;


void MainWindow::InitColorTable_grey()
{
    tableRGB.clear();
    Tif_RGB white(255, 255, 255);//第一阶颜色
    Tif_RGB black(0, 0, 0);//第二阶颜色
    vector<Tif_RGB> RGBList(256);
    Gradient(white, black, RGBList);
    for (int i = 0; i < 256; i++)
    {
        tableRGB.push_back(RGBList[i]);//1-256
    }
}

void MainWindow::InitColorTable_color(QString fileName)
{
    tableRGB.clear();
    QByteArray buffer = fileName.toLocal8Bit();
    const char *fileName_Char = buffer.constData();
    ifstream Color_file(fileName_Char);
    string f;
    float color[5][5];
    int count = 0;
    int i = 0;
    while (Color_file >> f)
    {
        color[i][count] = stof(f);
        if (count == 4)
        {
            i++;
            count = 0;
        }
        else
        count++;
    }
    Tif_RGB blue(color[0][1], color[0][2], color[0][3]);//第一阶颜色
    Tif_RGB green(color[1][1], color[1][2], color[1][3]);//第二阶颜色
    vector<Tif_RGB> RGBList(64);
    Gradient(blue, green, RGBList);
    for (int i = 0; i < 64; i++)
    {
        tableRGB.push_back(RGBList[i]);//1-64
    }

    Tif_RGB yellow(color[2][1], color[2][2], color[2][3]);//第三阶颜色
    RGBList.clear();
    RGBList.resize(64);
    Gradient(green, yellow, RGBList);
    for (int i = 0; i < 64; i++)
    {
        tableRGB.push_back(RGBList[i]);//65-128
    }

    Tif_RGB red(color[3][1], color[3][2], color[3][3]);//第四阶颜色
    RGBList.clear();
    RGBList.resize(64);
    Gradient(yellow, red, RGBList);
    for (int i = 0; i < 64; i++)
    {
        tableRGB.push_back(RGBList[i]);//129-193
    }

    Tif_RGB white(color[4][1], color[4][2], color[4][3]);//第五阶颜色
    RGBList.clear();
    RGBList.resize(64);
    Gradient(red, white, RGBList);
    for (int i = 0; i < 64; i++)
    {
        tableRGB.push_back(RGBList[i]);//194-256
    }
}


int GetColorIndex(float value, float min_value, float max_value)
{
    int Index = floor((value - min_value) * 255 / (max_value - min_value));
    return Index;
}


int MainWindow::Image_output(float Zenith_rad,float Azimuth_rad,float alpha)
{
    double time_Start = (double)clock();
    //加载待处理Tif文件，并获取文件的第一个波段。
    int Tif_Width = Tif_Data->GetRasterXSize();//图像宽度
    int Tif_Height = Tif_Data->GetRasterYSize();//图像高度
    //int Tif_BandNum = Tif_Data->GetRasterCount();//波段数
    GDALRasterBand* Tif_Band = Tif_Data->GetRasterBand(1);


    //创建图像驱动，配置图像信息。
    GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName("GTIFF"); //图像驱动
    char** ppszOptions = NULL;
    ppszOptions = CSLSetNameValue(ppszOptions, "BIGTIFF", "IF_NEEDED"); //配置图像信息


    //创建山地阴影输出文件。
    //const char* Hillshade_Path = "E:\\Hillshade.tif";
    int Hillshade_BandNum = 3;
    GDALDataset* Hillshade_Data = pDriver->Create(Hillshade_Path, Tif_Width, Tif_Height, Hillshade_BandNum, GDT_Byte, ppszOptions);
    if (Hillshade_Data == nullptr)
    {
        cout << "无法创建山地阴影Tif图像！" << endl;
        return false;
    }

    //声明待处理Tif文件值的Buffer，并设置变量的大小。
    float* Tif_Buffer = new float[Tif_Width * Tif_Height];


    //读取图像中的所有像元。
    Tif_Band->RasterIO(GF_Read, 0, 0, Tif_Width, Tif_Height, Tif_Buffer, Tif_Width, Tif_Height, GDT_Float32, 0, 0);


//    //为生成的Hillshade_Data添加地理参考信息。
//    OGRSpatialReference spatialReference;
//    spatialReference.importFromEPSG(4326);//EPSG432为wgs84地理坐标系
//    char* pszWKT = nullptr;
//    spatialReference.exportToWkt(&pszWKT);
//    Hillshade_Data->SetProjection(pszWKT);
//    CPLFree(pszWKT);
//    pszWKT = nullptr;
//    double padfTransform[6] =
//    {
//        adfGeoTransform[0],//左上角点坐标X
//        adfGeoTransform[1],//X方向的分辨率
//        adfGeoTransform[2],//旋转系数，如果为0，就是标准的正北向图像
//        adfGeoTransform[3],//左上角点坐标Y
//        adfGeoTransform[4],//旋转系数，如果为0，就是标准的正北向图像
//        adfGeoTransform[5],//Y方向的分辨率
//    };//坐标信息
//    Hillshade_Data->SetGeoTransform(padfTransform);


    //遍历Tif数据所有像元，选出最大值与最小值。
    float hMin = 3.402823466e+38F;
    float hMax = -3.402823466e+38F;
    double noValue = Tif_Data->GetRasterBand(1)->GetNoDataValue();
    for (int y = 1; y < Tif_Height - 1; y++)
    {
        for (int x = 1; x < Tif_Width - 1; x++)
        {
            unsigned int e = (unsigned int)Tif_Width * y + x;
            if (Tif_Buffer[e] < hMin) hMin = Tif_Buffer[e];
            if (Tif_Buffer[e] > hMax) hMax = Tif_Buffer[e];
        }
    }

    //声明保存HillShade的Buffer，并设置变量的大小。
    BYTE* HillShade_Buffer = new BYTE[Tif_Width * Tif_Height * 3];
    memset(HillShade_Buffer, 0, Tif_Width * Tif_Height * 3 * sizeof(BYTE));


    //初始化色带表，并输出色带至tableRGB。
    //InitColorTable();


    //通过两个循环输出山地阴影数据到新建Tif数据。
    ui->progressBar->setMaximum(Tif_Data->GetRasterYSize()-2);
    //ui->progressBar->setFormat("绘制中...");
    for (int y = 1; y < Tif_Height - 1; y++)
    {
        for (int x = 1; x < Tif_Width - 1; x++)
        {
            unsigned int e = (unsigned int)Tif_Width * y + x;
            unsigned int f = e + 1;
            unsigned int d = e - 1;

            unsigned int b = e - Tif_Width;
            unsigned int c = b + 1;
            unsigned int a = b - 1;

            unsigned int h = e + Tif_Width;
            unsigned int i = h + 1;
            unsigned int g = h - 1;

            double Hillshade_R;
            double Hillshade_G;
            double Hillshade_B;

            if (Tif_Buffer[a] == 0 && Tif_Buffer[b] == 0 && Tif_Buffer[c] == 0 && Tif_Buffer[d] == 0 &&
                Tif_Buffer[e] == 0 && Tif_Buffer[f] == 0 && Tif_Buffer[g] == 0 && Tif_Buffer[h] == 0 && Tif_Buffer[i] == 0)
            {
                Hillshade_R = noValue;
                Hillshade_G = noValue;
                Hillshade_B = noValue;
            }
            else
            {
                float Hillshade = Calculate_hillshade(Tif_Buffer[a], Tif_Buffer[b], Tif_Buffer[c], Tif_Buffer[d],
                     Tif_Buffer[f], Tif_Buffer[g], Tif_Buffer[h], Tif_Buffer[i], Zenith_rad, Azimuth_rad);

                int index = GetColorIndex(Tif_Buffer[e], hMin, hMax);

                Hillshade_R = Hillshade * alpha + (1 - alpha) * tableRGB[index].R;
                Hillshade_G = Hillshade * alpha + (1 - alpha) * tableRGB[index].G;
                Hillshade_B = Hillshade * alpha + (1 - alpha) * tableRGB[index].B;
            }
            HillShade_Buffer[3 * e + 0] = Hillshade_R;
            HillShade_Buffer[3 * e + 1] = Hillshade_G;
            HillShade_Buffer[3 * e + 2] = Hillshade_B;
        }
        ui->progressBar->setValue(y);
        ui->progressBar->update();
        qApp->processEvents();
    }
    ui->textBrowser_Feedback->append("已处理");
    ui->textBrowser_Feedback->insertPlainText(QString::number(Tif_Data->GetRasterYSize()));
    ui->textBrowser_Feedback->insertPlainText("行像素，");

    Hillshade_Data->RasterIO(GF_Write, 0, 0, Tif_Width, Tif_Height, HillShade_Buffer, Tif_Width, Tif_Height, GDT_Byte, 3, NULL, 3, 3 * Tif_Width, 1);
    double time_Finish = (double)clock();

    ui->textBrowser_Feedback->insertPlainText("耗时");
    ui->textBrowser_Feedback->insertPlainText(QString::number((time_Finish-time_Start)/1000));
    ui->textBrowser_Feedback->insertPlainText("秒,");
    ui->textBrowser_Feedback->insertPlainText("山地阴影已输出完毕");
    ui->textBrowser_Feedback->append("图像已输出至");
    ui->textBrowser_Feedback->insertPlainText(Hillshade_Path);
    ui->textBrowser_Feedback->append("-------------------------------------------");


    delete[] Tif_Buffer;
    delete[] HillShade_Buffer;

    GDALClose(Hillshade_Data);
    GDALClose(Tif_Data);

    return 0;
}


float MainWindow::Calculate_hillshade(float Tif_Buffer_A, float Tif_Buffer_B, float Tif_Buffer_C, float Tif_Buffer_D,
    float Tif_Buffer_F, float Tif_Buffer_G, float Tif_Buffer_H, float Tif_Buffer_I, float Zenith_rad, float Azimuth_rad)
{
    float TempHillshade;
    float dz_dx = Calculate_dz_dx(Tif_Buffer_A, Tif_Buffer_C, Tif_Buffer_D, Tif_Buffer_F, Tif_Buffer_G, Tif_Buffer_I);
    float dz_dy = Calculate_dz_dy(Tif_Buffer_A, Tif_Buffer_B, Tif_Buffer_C, Tif_Buffer_G, Tif_Buffer_H, Tif_Buffer_I);
    float Slope_rad = Calculate_Slope_rad(dz_dx, dz_dy);
    float Aspect_rad = Calculate_Aspect_rad(dz_dx, dz_dy);

    TempHillshade = 255.0 * ((cos(Zenith_rad) * cos(Slope_rad)) + (sin(Zenith_rad) * sin(Slope_rad) * cos(Azimuth_rad - Aspect_rad)));
    if (TempHillshade <= 0)TempHillshade = 1;

    return TempHillshade;
}


float MainWindow::Calculate_dz_dx(float Tif_Buffer_A, float Tif_Buffer_C, float Tif_Buffer_D, float Tif_Buffer_F, float Tif_Buffer_G, float Tif_Buffer_I)//计算x方向上的变化率
{
    float dz_dx;

    dz_dx = ((Tif_Buffer_C + 2 * Tif_Buffer_F + Tif_Buffer_I) -
        (Tif_Buffer_A + 2 * Tif_Buffer_D + Tif_Buffer_G)) / (8 * cellsize);

    return dz_dx;
}


float MainWindow::Calculate_dz_dy(float Tif_Buffer_A, float Tif_Buffer_B, float Tif_Buffer_C, float Tif_Buffer_G, float Tif_Buffer_H, float Tif_Buffer_I)//计算y方向上的变化率
{
    float dz_dy;

    dz_dy = ((Tif_Buffer_G + 2 * Tif_Buffer_H + Tif_Buffer_I) -
        (Tif_Buffer_A + 2 * Tif_Buffer_B + Tif_Buffer_C)) / (8 * cellsize);

    return dz_dy;
}


float MainWindow::Calculate_Slope_rad(float dz_dx,float dz_dy)//计算坡度弧度
{
    float Slope_rad = 0;
    Slope_rad = atan(z_factor * sqrt((dz_dx) * (dz_dx)+(dz_dy) * (dz_dy)));
    return Slope_rad;
}


float MainWindow::Calculate_Aspect_rad(float dz_dx, float dz_dy)//计算坡向弧度
{
    float Aspect_rad = 0;
    if (dz_dx != 0)
    {
        Aspect_rad = atan2(dz_dy, -(dz_dx));
        if (Aspect_rad < 0)
        {
            Aspect_rad = 2 * pi + Aspect_rad;
        }
    }
    else if (dz_dx == 0)
    {
        if (dz_dy > 0)
        {
            Aspect_rad = pi / 2;
        }
        else if (dz_dy < 0)
        {
            Aspect_rad = 2 * pi - pi / 2;
        }
        else
        {
            Aspect_rad = 0;
        }
    }
    return Aspect_rad;
}


void MainWindow::tableRGB_output()
{
    GDALAllRegister();
    //创建图像驱动，配置图像信息。
    GDALDriver* pDriver2 = GetGDALDriverManager()->GetDriverByName("GTIFF"); //图像驱动
    char** ppszOptions2 = NULL;
    ppszOptions2 = CSLSetNameValue(ppszOptions2, "BIGTIFF", "IF_NEEDED"); //配置图像信息

    //创建tableRGB输出文件。
    QString tableRGB_Path_new;
    QString tableRGB_Path_tmp;
    tableRGB_Path_tmp=Tiff_fileName;
    QString tableRGB_name="_tableRGB.tif";
    tableRGB_Path_tmp.chop(4);
    tableRGB_Path_new=tableRGB_Path_tmp+tableRGB_name;
    QByteArray buffer = tableRGB_Path_new.toLatin1();
    tableRGB_Path = buffer.data();

    ui->textBrowser_Feedback->append("色带已输出至");
    ui->textBrowser_Feedback->insertPlainText(tableRGB_Path);
    ui->textBrowser_Feedback->append("-------------------------------------------");

    //tableRGB_Path = "E:\\tableRGB.tif";
    int tableRGB_BandNum = 3;
    GDALDataset* tableRGB_Data = pDriver2->Create(tableRGB_Path, 256, 10, tableRGB_BandNum, GDT_Byte, ppszOptions2);
    GDALRasterBand* tableRGB_Band1 = tableRGB_Data->GetRasterBand(1);
    GDALRasterBand* tableRGB_Band2 = tableRGB_Data->GetRasterBand(2);
    GDALRasterBand* tableRGB_Band3 = tableRGB_Data->GetRasterBand(3);

    //声明保存tableRGB的Buffer，并设置变量的大小。
    float* tableRGB_Buffer = new float[256];
    memset(tableRGB_Buffer, 0, 256 * sizeof(float));

    //输出色带至tableRGB。
    for (int i = 0; i < 256; i++)
    {
        tableRGB_Buffer[i] = tableRGB[i].R;
        tableRGB_Band1->RasterIO(GF_Write, 0, 0, 256, 10, tableRGB_Buffer, 256, 1, GDT_Float32, 0, 0);
    }
    for (int i = 0; i < 256; i++)
    {
        tableRGB_Buffer[i] = tableRGB[i].G;
        tableRGB_Band2->RasterIO(GF_Write, 0, 0, 256, 10, tableRGB_Buffer, 256, 1, GDT_Float32, 0, 0);
    }
    for (int i = 0; i < 256; i++)
    {
        tableRGB_Buffer[i] = tableRGB[i].B;
        tableRGB_Band3->RasterIO(GF_Write, 0, 0, 256, 10, tableRGB_Buffer, 256, 1, GDT_Float32, 0, 0);
    }

    delete[] tableRGB_Buffer;
    GDALClose(tableRGB_Data);
}
