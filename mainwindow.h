#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include"globaldata.h"
#include <QMainWindow>
#include"dealusb_msg.h"
#include"savepcdthread.h"
#include"statisticsdialog.h"
#include"filesave_dialog.h"
#include"savepcdthread.h"
#include"calmeanstdthread.h"
#include"autocalibration_dialog.h"
#include<receuv910.h>
#include"aboutdialog.h"
#include "camerasetting_dialog.h"
#include"blackgetcount.h"
#include"rawdataui_dialog.h"
#include"hist_ma_dialog.h"
#include"calma_thread.h"
#include"QFile"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *e);

    void init_thread();    //线程的初始化
    void init_connect();   //信号与槽的初始化
    void load_ini_file();   //加载配置文件

private slots:

    void Load();

    void EnumDevice();

    void Display_log_slot(QString str);  //打印日志信息到控制信息输出窗口的槽函数

    void on_openFile_action_triggered();  //打开 本地文件TOF\PEAK

    void on_play_pushButton_clicked();   //播放 3D、2D  ，打开相关的定时器

    void show_image_timer_slot();       //播放2D图像的槽函数

    void queryPixel_showToolTip_slot(int x,int y);  //鼠标停靠处显示TOF 和 peak信息

    void on_tof_peak_change_toolButton_clicked();

    void on_gain_lineEdit_returnPressed();

    void on_filter_radioButton_clicked();

    void on_statistic_action_triggered();

    void on_saveFile_action_triggered();

    void isSaveFlagSlot(bool,QString,int);       //是否保存标识；存储路径；文件类型（预留）

    void on_rotate_horizontalSlider_sliderMoved(int position);    //旋转

    void on_scale_horizontalSlider_sliderMoved(int position);     //缩放

    void on_translate_horizontalSlider_sliderMoved(int position); //平移

    void save3DSettingFile();

    void on_peakOffset_lineEdit_returnPressed();

    void on_averageNum_lineEdit_returnPressed();

    void on_centerShowYes_radioButton_clicked();

    void on_centerShowNo_radioButton_clicked();

    void on_AutoCalibration_action_triggered();

    void oneSec_timer_slot();    //统计帧率的槽函数

    void rece_oneFrame_slot();



    void on_newFrame_pushButton_clicked();
//    void on_pushButton_2_clicked();    //尝试切换模式的


    // ******************* UV910 相关***********************/
    void enable_UV910_trueOrFalse(bool flag);
    void init_UV910();

    bool OpenCamera();
    bool CloseCamera();
    bool bOpenDevice();
    int  EnumerateDothinkeyDevice();
//    void SetDeviceName(QString pDeviceName);
    void GetPmuCurrent();
    void GetADValue();
    void Sleepms(double milsecond);//进准计时

    void on_load_iniFile_pushButton_clicked();

    void on_openDevice_pushButton_clicked();

    void link_UV910_return_slot(bool);   //910连接的返回信息的槽函数



    void on_about_action_triggered();

    void on_front_toolButton_clicked();

    void on_side_toolButton_clicked();

    void on_down_toolButton_clicked();

    void on_I2C_read_pushButton_clicked();

    void on_I2C_write_pushButton_clicked();

    void on_kalman_checkBox_clicked();

    void on_pileUp_checkBox_clicked();

    void on_gain_peak_lineEdit_returnPressed();


    void on_cameraPara_action_triggered();

    void on_kalmanPara_lineEdit_returnPressed();

    void on_meanTof_offset_lineEdit_returnPressed();

    void on_balckileSave_toolButton_clicked();

    void on_blackStart_pushButton_clicked();

    void on_RawData_action_triggered();





    /*******DCR测试相关*****/
    void sendSavedFrame_slot(int);

    /*********接收rawData相关*******/
    void on_start_rawDataSave_slot(QString );

    void on_Hist_MA_action_triggered();

    /*****接收RowData MA 相关***********/
    void startReceUV910_rowData_MA_slot(bool);






    void on_isShowNormalizationPeak_checkBox_clicked();

    void on_black_rowAndCol_toolButton_clicked();

    void on_rawDatapushButton_clicked();

    void on_autoCalibration_action_triggered();


    /**************************/
    void write_I2C_slot(QString addressStr,QString valueStr);

signals:
    void rece_signal();

    void loadLocalArray_signal();
    void change_gain_signal(float);
    void change_tof_peak_signal();
    void isFilter_signal(bool);
    void receUV910_data_signal();  //开启数据接收的信

    void test_start();

    void start_openUV910_signal(QString filePath); //打开UV910设备信号
    void stop_UV910_signal();                      //关闭UV910设备信号


    void sendPlayLocal_signal(QString);

    //DCR测量相关
    void start_blackGetCount_signal(QString filePath,QString InteTime,int saveFrame,QStringList rowAndCol);



private:

    receUV910 *receUV910_obj;
    QThread *receUV910_thread;

    QThread *dealMsg_thread;      //数据处理线程
    DealUsb_msg *dealMsg_obj;

    QThread *savePcd_thread;      //文件保存线程
    savePCDThread* savePcd_obj;

    calMeanStdThread *calMeanStd_obj; //计算统计信息的线程
    QThread *calThread;

    QString localFileDirPath;    //本地播放文件
    statisticsDialog  statisticsDia_; //统计信息界面
    fileSave_Dialog fileSave_dia;         //文件保存界面
    autoCalibration_Dialog autoCalibration_dia;  //自动校准界面
    QTimer show_image_timer;         //2D图像的刷新定时器
    QTimer oneSec_timer;            //统计帧率相关
    int frameCount;
    QLabel fpsLabel;

    aboutDialog about_dia;

    CameraSetting_Dialog  cameraSetting_dia;


    //UV910 相关
    char* pDeviceName[8];
    bool m_isTV;
    int vpp;
    int afvcc;
    int m_PreviewWidth;
    int m_PreviewHeight;
    unsigned long m_GrabSize;


    //////////暗计数相关///////////////////////
    blackGetCount *blackGetCount_obj;
    QThread *blackGetCount_thread;
    QStringList rowAndCol_strlist;


    //////rawData数据保存相关//////////
    /*********保存rowData信息槽函数*************/
    rawDataUI_Dialog rawData_dia;

    /***********RowData MA 相关**********/
    Hist_MA_Dialog Hist_MA_dia;
    calMA_thread *calMA_obj;
    QThread *calMA_the_thread;


    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
