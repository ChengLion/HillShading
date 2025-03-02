#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "gdal.h"
#include "gdal_priv.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_load_clicked();
    void on_pushButton_begin_clicked();
    void on_radioButton_clicked(bool checked);
    void on_radioButton_2_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    const char* Tif_Path;
    GDALDataset* Tif_Data;
    const char* Hillshade_Path;
    const char* tableRGB_Path;
    double alpha;
    QString Tiff_fileName;

    void initUi();
    void DisplayTifImage(const char* Tif_Path);
    void DisplayRGBTable(const char* Path);
    void DisplayTifInfo(GDALDataset* Tif_Data);
    void InitColorTable_color(QString fileName);
    void InitColorTable_grey();
    int Image_output(float Zenith_rad,float Azimuth_rad,float alpha);
    float Calculate_hillshade(float Tif_Buffer_A, float Tif_Buffer_B, float Tif_Buffer_C, float Tif_Buffer_D, float Tif_Buffer_F, float Tif_Buffer_G, float Tif_Buffer_H, float Tif_Buffer_I, float Zenith_rad, float Azimuth_rad);
    float Calculate_dz_dx(float Tif_Buffer_A, float Tif_Buffer_C, float Tif_Buffer_D, float Tif_Buffer_F, float Tif_Buffer_G, float Tif_Buffer_I);
    float Calculate_dz_dy(float Tif_Buffer_A, float Tif_Buffer_B, float Tif_Buffer_C, float Tif_Buffer_G, float Tif_Buffer_H, float Tif_Buffer_I);
    float Calculate_Slope_rad(float dz_dx, float dz_dy);
    float Calculate_Aspect_rad(float dz_dx, float dz_dy);
    void reuse();
    void tableRGB_output();
};
#endif // MAINWINDOW_H
