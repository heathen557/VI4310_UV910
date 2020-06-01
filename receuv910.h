#ifndef RECEUV910_H
#define RECEUV910_H

#include <QObject>
#include"globaldata.h"

class receUV910 : public QObject
{
    Q_OBJECT
public:
    explicit receUV910(QObject *parent = 0);

    unsigned long m_GrabSize;



    //UV910 相关
    char* pDeviceName[8];
    bool m_isTV;
    int vpp;
    int afvcc;
    int m_PreviewWidth;
    int m_PreviewHeight;
//    unsigned long m_GrabSize;


    /*********接收Rowdata数据相关***************/
    int isRawDataSave_flag;      // 1:rowdata数据保存   2：RowData MA数据接收

signals:

    void rece_oneFrame_signal();

    void UV910_data_deal_signal(QByteArray);

    void UV910_Qtech_deal_signal(QByteArray);

    void link_UV910_return_signal(bool);   //打开的返回信号   true:打开成功   false:失败

    //rawData信号
    void UV910_RawData_deal_signal(QByteArray);
    //rowData MA 信号
    void UV910_RawData_MA_deal_signal(QByteArray);

public slots:

    void receUV910_data_slot();


    void init_UV910();
//    void on_load_iniFile_pushButton_clicked();
    bool bloadIniFile(QString sFile);
    void Sleepms(double milsecond);
    bool OpenCamera();
    bool bOpenDevice();
    void on_newFrame_pushButton_clicked();
    void GetADValue();
    bool CloseCamera();
    void EnumDevice();
    int EnumerateDothinkeyDevice();
    void GetPmuCurrent();
    void Play();
    void Load(QString filePath);
//    void on_openDevice_pushButton_clicked();
    void on_openDevice_pushButton_clicked(QString filePath);

    //接收rowdata isRawDataSave_flag = 1;
    void startReceUV910_rowData_slot(bool);

    //接收rowdataMA 的模式 isRawDataSave_flag = 2;
    void startReceUV910_rowData_MA_slot(bool);



};

#endif // RECEUV910_H
