#include "mainwindow.h"
#include "ui_mainwindow.h"


using namespace std;


//图像显示相关
extern QMutex mutex_3D;  //3D点云/2D传输的互斥锁
extern QImage tofImage;
extern QImage intensityImage;
extern pcl::PointCloud<pcl::PointXYZRGB> pointCloudRgb;
extern bool isShowPointCloud;  //是否有点云数据 ，有的话显示否则不显示

//鼠标点击显示时相关
//鼠标点击显示时相关
extern QMutex mouseShowMutex;
extern float mouseShowTOF[160][120];
extern float mouseShowPEAK[160][120];
extern float mouseShowDepth[160][120];
extern float mouseShow_X[160][120];
extern float mouseShow_Z[160][120];

/*保存用到的标识*/
extern bool isSaveFlag;        //是否进行存储
extern QString saveFilePath;   //保存的路径  E:/..../.../的形式
extern int saveFileIndex;      //文件标号；1作为开始

/**UV910 相关**/
extern SensorTab CurrentSensor;
extern int m_RunMode;
extern int m_nDevID;
extern bool isReceUV910_flag;  //是否接收数据的标识





MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //    ui->RawData_action->setVisible(false);
    //    ui->Hist_MA_action->setVisible(false);

    qRegisterMetaType<QVector<double>>("QVector<double>");   //注册函数
    qRegisterMetaType<QVector<int>>("QVector<int>");      //注册函数

    ui->config_toolBox->setCurrentIndex(0);
    frameCount = 0;   //统计帧率时候用

    ui->groupBox_5->setVisible(false);

    temperatureIndex = 0;
    ui->statusBar->addWidget(&fpsLabel);
    ui->statusBar->addWidget(&temptureLabel);

#ifndef SHOW_HISTORGRAM_BUTTON
    ui->tof_Histogram_pushButton->setVisible(false);
    ui->peak_Histogram_pushButton->setVisible(false);
#endif
#ifndef SHOW_TOF_PEAK_BUTTON
    ui->tof_peak_change_toolButton->setVisible(false);
#endif

    init_UV910();   // 初始化uv910
    init_thread();
    init_connect();
    load_ini_file();

    connect(this,SIGNAL(rece_signal()),receUV910_obj,SLOT(receUV910_data_slot()));

    emit loadLocalArray_signal();

}


MainWindow::~MainWindow()
{
    delete ui;
    if(CurrentSensor.ParaList!=nullptr)
    {
        delete CurrentSensor.ParaList;
        CurrentSensor.ParaList=nullptr;
    }
}

//!
//! \brief MainWindow::init_thread
//!   //线程的初始化
void MainWindow::init_thread()
{
    receUV910_obj = new receUV910;
    receUV910_thread = new QThread;
    receUV910_obj->moveToThread(receUV910_thread);
    receUV910_thread->start();


    //数据接收线程
    dealMsg_thread = new QThread();
    dealMsg_obj = new DealUsb_msg();
    dealMsg_obj->moveToThread(dealMsg_thread);
    dealMsg_thread->start();

    //文件保存线程
    savePcd_thread = new QThread();      //文件保存线程
    savePcd_obj = new savePCDThread;
    savePcd_obj->moveToThread(savePcd_thread);
    savePcd_thread->start();

    //统计信息相关
    calThread = new QThread();
    calMeanStd_obj = new calMeanStdThread(); //
    calMeanStd_obj->moveToThread(calThread);
    calThread->start();

    //暗计数相关
    blackGetCount_obj = new blackGetCount();
    blackGetCount_thread = new QThread();
    blackGetCount_obj->moveToThread(blackGetCount_thread);
    blackGetCount_thread->start();

    //计算MA 相关
    calMA_obj = new calMA_thread;
    calMA_the_thread = new QThread;
    calMA_obj->moveToThread(calMA_the_thread);
    calMA_the_thread->start();
}

//!
//! \brief MainWindow::init_connect
//!
void MainWindow::init_connect()   //信号与槽的初始化
{
    //与数据处理线程的 信号与槽的连接
    connect(dealMsg_obj,&DealUsb_msg::Display_log_signal,this,&MainWindow::Display_log_slot);
    connect(this,&MainWindow::loadLocalArray_signal,dealMsg_obj,&DealUsb_msg::loadLocalArray);
    connect(this,&MainWindow::change_gain_signal,dealMsg_obj,&DealUsb_msg::change_gain_slot);
    connect(this,&MainWindow::change_tof_peak_signal,dealMsg_obj,&DealUsb_msg::change_tof_peak_slot);
    connect(this,&MainWindow::isFilter_signal,dealMsg_obj,&DealUsb_msg::isFilter_slot);


    connect(this,&MainWindow::sendPlayLocal_signal,dealMsg_obj,&DealUsb_msg::playLocalFile_slot);

    //文件保存相关的信号与槽的连接
    connect(&fileSave_dia,&fileSave_Dialog::isSaveFlagSignal,this,&MainWindow::isSaveFlagSlot);
    connect(&fileSave_dia,&fileSave_Dialog::alter_fileSave_signal,dealMsg_obj,&DealUsb_msg::alter_fileSave_slot);
    connect(dealMsg_obj,&DealUsb_msg::saveTXTSignal,savePcd_obj,&savePCDThread::saveTXTSlot);
    connect(savePcd_obj,&savePCDThread::send_savedFileIndex_signal,&fileSave_dia,&fileSave_Dialog::send_savedFileIndex_slot);


    //统计信息相关的槽函数
    connect(calMeanStd_obj,SIGNAL(statistic_MeanStdSignal(QStringList,QStringList,QStringList,QStringList)),&statisticsDia_,SLOT(statistic_MeanStdSlot(QStringList,QStringList,QStringList,QStringList)));
    connect(&statisticsDia_,SIGNAL(startStop_signal(int)),calMeanStd_obj,SLOT(startStop_slot(int)));
    connect(&statisticsDia_,SIGNAL(alterStatisticFrameNum_signal(int)),dealMsg_obj,SLOT(alterStatisticFrameNum_slot(int)));

    //2D图像的显示相关
    connect(&show_image_timer,SIGNAL(timeout()),this,SLOT(show_image_timer_slot()));

    //鼠标停靠处显示
    connect(ui->tof_label,SIGNAL(queryPixSignal(int,int)),this,SLOT(queryPixel_showToolTip_slot(int,int)));
    connect(ui->peak_label,SIGNAL(queryPixSignal(int,int)),this,SLOT(queryPixel_showToolTip_slot(int,int)));

    //数据接收线程相关的
    connect(this,SIGNAL(receUV910_data_signal()),receUV910_obj,SLOT(receUV910_data_slot()));
    connect(receUV910_obj,SIGNAL(rece_oneFrame_signal()),this,SLOT(rece_oneFrame_slot()));
    connect(receUV910_obj,SIGNAL(UV910_Qtech_deal_signal(QByteArray)),dealMsg_obj,SLOT(UV910_Qtech_deal_slot(QByteArray)));
    connect(this,SIGNAL(start_openUV910_signal(QString)),receUV910_obj,SLOT(on_openDevice_pushButton_clicked(QString)));
    connect(this,SIGNAL(stop_UV910_signal()),receUV910_obj,SLOT(CloseCamera()));
    connect(receUV910_obj,SIGNAL(link_UV910_return_signal(bool)),this,SLOT(link_UV910_return_slot(bool)));
    //    connect(this,SIGNAL(test_start()),receUV910_obj,SLOT(on_openDevice_pushButton_clicked()));
    connect(receUV910_obj,&receUV910::write_I2C_integrate_signal,this,&MainWindow::write_I2C_slot);

    //统计帧率
    connect(&oneSec_timer,SIGNAL(timeout()),this,SLOT(oneSec_timer_slot()));

    //自动校准相关
    connect(&autoCalibration_dia,SIGNAL(start_autoCalibration_signal(float)),dealMsg_obj,SLOT(start_autoCalibration_slot(float)));
    connect(dealMsg_obj,SIGNAL(send_cali_success_signal(QString)),&autoCalibration_dia,SLOT(send_cali_success_slot(QString)));

    //相机配置  修改相机焦距、积分次数、相机镜头间距
    connect(&cameraSetting_dia,SIGNAL(alter_focal_integrate_signal(float,float,int)),dealMsg_obj,SLOT(alter_focal_integrate_slot(float,float,int)));
    connect(&cameraSetting_dia,&CameraSetting_Dialog::write_I2C_integrate_signal,this,&MainWindow::write_I2C_slot);


    //DCR 测量相关
    connect(this,&MainWindow::start_blackGetCount_signal,blackGetCount_obj,&blackGetCount::start_blackGetCount_slot);
    connect(blackGetCount_obj,&blackGetCount::sendSavedFrame_signal,this,&MainWindow::sendSavedFrame_slot);


    //RowData 数据相关
    connect(receUV910_obj,&receUV910::UV910_RawData_deal_signal,dealMsg_obj,&DealUsb_msg::UV910_RawData_deal_slot);
    connect(&rawData_dia,&rawDataUI_Dialog::on_start_rawDataSave_signal,dealMsg_obj,&DealUsb_msg::on_start_rawDataSave_slot);
    connect(&rawData_dia,SIGNAL(on_start_rawDataSave_signal(QString)),this,SLOT(on_start_rawDataSave_slot(QString)));
    connect(savePcd_obj,&savePCDThread::send_savedFileIndex_signal,&rawData_dia,&rawDataUI_Dialog::send_savedFileIndex_slot);
    connect(&rawData_dia,&rawDataUI_Dialog::startReceUV910_rowData_signal,receUV910_obj,&receUV910::startReceUV910_rowData_slot);

    //RawData MA 数据相关
    connect(&Hist_MA_dia,&Hist_MA_Dialog::startReceUV910_rowData_MA_signal,receUV910_obj,&receUV910::startReceUV910_rowData_MA_slot);  //
    connect(&Hist_MA_dia,&Hist_MA_Dialog::startReceUV910_rowData_MA_signal,this,&MainWindow::startReceUV910_rowData_MA_slot);
    connect(receUV910_obj,&receUV910::UV910_RawData_MA_deal_signal,dealMsg_obj,&DealUsb_msg::UV910_RawData_MA_deal_slot);
    connect(dealMsg_obj,&DealUsb_msg::send_calMA_signal,calMA_obj,&calMA_thread::send_calMA_slot);
    connect(&Hist_MA_dia,&Hist_MA_Dialog::start_RowData_bin_histogram_signal,calMA_obj,&calMA_thread::start_RowData_bin_histogram_slot);
    connect(calMA_obj,&calMA_thread::toShowHistogram_channel1_signal,&Hist_MA_dia,&Hist_MA_Dialog::toShowHistogram_channel1_slot);
    connect(calMA_obj,&calMA_thread::toShowHistogram_channel2_signal,&Hist_MA_dia,&Hist_MA_Dialog::toShowHistogram_channel2_slot);
    connect(calMA_obj,&calMA_thread::toShowHistogram_channel3_signal,&Hist_MA_dia,&Hist_MA_Dialog::toShowHistogram_channel3_slot);
    connect(calMA_obj,&calMA_thread::toShowHistogram_channel4_signal,&Hist_MA_dia,&Hist_MA_Dialog::toShowHistogram_channel4_slot);
    connect(calMA_obj,&calMA_thread::sendFrameIndex_signal,&Hist_MA_dia,&Hist_MA_Dialog::sendFrameIndex_slot);
    connect(&Hist_MA_dia,&Hist_MA_Dialog::clearHistogram_signal,calMA_obj,&calMA_thread::clearHistogram_slot);

}




//!
//! \brief MainWindow::load_ini_file
//!加载配置集文件
void MainWindow::load_ini_file()
{
    QSettings configSetting("setting.ini", QSettings::IniFormat);

    int Rotate_rate = configSetting.value("operation/Rotate_rate").toInt();
    int Scale_rate = configSetting.value("operation/Scale_rate").toInt();
    int transtate_rate = configSetting.value("operation/transtate_rate").toInt();


    QString iniFile_path = configSetting.value("init/filepath").toString();
    ui->initFilePath_lineEdit->setText(iniFile_path);


    if(Rotate_rate==0 || Scale_rate==0 || transtate_rate==0)    //防止第一次加载时，没有配置文件，这时候初始化为初始值
    {
        Rotate_rate = 8;
        Scale_rate = 10;
        transtate_rate = 30;
        QString log_str = "[load conf file error]:setting.ini";
        Display_log_slot(log_str);
    }

    ui->widget->rotateRate = Rotate_rate;
    ui->widget->scaleRate = Scale_rate;
    ui->widget->translateRate = 110-transtate_rate;


    ui->rotate_horizontalSlider->setValue(Rotate_rate);
    ui->scale_horizontalSlider->setValue(Scale_rate);
    ui->translate_horizontalSlider->setValue(transtate_rate);
    QString log_str = "[load conf file success]:setting.ini";
    Display_log_slot(log_str);

}


//!
//! \brief MainWindow::on_openFile_action_triggered
//!打开本地文件的槽函数  弹出窗口
void MainWindow::on_openFile_action_triggered()
{
    QString filePath=QFileDialog::getExistingDirectory();
    filePath.append("/");

    if(filePath.length()<2)
        return;

    localFileDirPath = filePath;
    QString log_str_ = "[Local Dir]:" + filePath;
    Display_log_slot(log_str_);

    dealMsg_obj->playLocalFile_slot(filePath);
    emit  sendPlayLocal_signal(filePath);

}

//!
//! \brief MainWindow::on_play_pushButton_clicked
//!开启播放图像的槽函数
void MainWindow::on_play_pushButton_clicked()
{
    //1 首先根据配置文件设置积分次数
    QSettings configSetting("setting.ini", QSettings::IniFormat);
    int integrate_num = configSetting.value("camera/integrate_num").toInt();

    QString integrateStr = QString("%1").arg(integrate_num,3,16,QChar('0')).toUpper();
    qDebug()<<"integrateStr = "<<integrateStr;
    QString highByte = integrateStr.mid(0,1) + "0";
    QString lowByte = integrateStr.mid(1,2);
    qDebug()<<"highByte="<<highByte<<"  lowByte="<<lowByte;

    QString addressStr = "07";
    write_I2C_slot(addressStr,highByte);
    Sleep(10);
    addressStr = "08";
    write_I2C_slot(addressStr,lowByte);


    if(ui->play_pushButton->text() == "play")
    {
        isReceUV910_flag = true;
        emit receUV910_data_signal();       //数据接收线程开始接收数据

        ui->widget->show3D_timer.start(90);   //3D点云的刷新频率
        show_image_timer.start(90);
        ui->play_pushButton->setText("pause");
    }else
    {
        isReceUV910_flag = false;
        ui->widget->show3D_timer.stop();
        show_image_timer.stop();
        ui->play_pushButton->setText("play");
    }
}


//!
//! \brief MainWindow::show_image_timer_slot
//! 播放2D图像的槽函数
void MainWindow::show_image_timer_slot()
{
    if(!isShowPointCloud)
        return;

    if(!tofImage.isNull() && !intensityImage.isNull())
    {
        mutex_3D.lock();
        QPixmap pixmap_tof(QPixmap::fromImage (tofImage));
        ui->tof_label->setPixmap(pixmap_tof);

        QPixmap pixmap_peak(QPixmap::fromImage (intensityImage));
        ui->peak_label->setPixmap(pixmap_peak);
        mutex_3D.unlock();
    }
}


//! \brief MainWindow::queryPixel_showToolTip_slot
//! \param x   实际的x坐标值  缩放以后的
//! \param y   实际的y坐标值  缩放以后的
//!  鼠标停靠处显示tof和peak
void MainWindow::queryPixel_showToolTip_slot(int x,int y)
{

    float width_scale = ui->tof_label->width()/160.0;
    float height_scale = ui->tof_label->height()/120.0;

    int y_index = y/height_scale;
    int x_index = x/width_scale;
    //    qDebug()<<"y_index="<<y_index<<"  x_index ="<<x_index;
    int index = 160*y_index + x_index;
    mouseShowMutex.lock();
    QString str= "x="+QString::number(x_index)+",y="+QString::number(y_index)+",tof="+QString::number(mouseShowTOF[x_index][y_index])+",peak="+QString::number(mouseShowPEAK[x_index][y_index])+
            +",X="+QString::number(mouseShow_X[x_index][y_index],'f',2) + "m,Y="+QString::number(mouseShowDepth[x_index][y_index],'f',2)+"m,Z=" +QString::number(mouseShow_Z[x_index][y_index],'f',2)+"m" ;
    mouseShowMutex.unlock();
    QToolTip::showText(QCursor::pos(),str);
}


//!
//! \brief MainWindow::Display_log_slot
//!打印控制信息的槽函数
//! 打印完日志读取I2C的 来获取积分次数
void MainWindow::Display_log_slot(QString str)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd");
    QString current_time = current_date_time.toString("hh:mm:ss.zzz ");
    str = current_time + str;

    ui->control_log_textEdit->append(str);


    //获取积分次数
    //    QString regStr1 = "07";
    //    QString regStr2 = "08";
    //    int iRet = 0;
    //    UINT uAddr = ui->I2C_addr_lineEdit->text().toInt(NULL,16);
    //    UINT uReg;
    //    USHORT uValue_high,uValue_low;

    //    uReg = regStr1.toInt(NULL,16);
    //    iRet = ReadSensorReg(uAddr,uReg,&uValue_high,0,m_nDevID);
    //    if(iRet != DT_ERROR_OK)
    //    {
    //        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("读取I2C,reg:07出错!"));
    //        return;
    //    }

    //    uReg = regStr2.toInt(NULL,16);
    //    iRet = ReadSensorReg(uAddr,uReg,&uValue_low,0,m_nDevID);
    //    if(iRet != DT_ERROR_OK)
    //    {
    //        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("读取I2C,reg:08出错!"));
    //    }
    //    int jifen_number = uValue;


}



//!
//! \brief MainWindow::on_tof_peak_change_toolButton_clicked
//!切换tof peak的槽函数
void MainWindow::on_tof_peak_change_toolButton_clicked()
{
    emit change_tof_peak_signal();
}


//!
//! \brief MainWindow::on_gain_lineEdit_returnPressed
//!改变tof增益的槽函数
void MainWindow::on_gain_lineEdit_returnPressed()
{
    float gain = ui->gain_lineEdit->text().toFloat();
    emit change_gain_signal(gain);
    dealMsg_obj->gainImage_tof = gain;
}

//!
//! \brief MainWindow::on_gain_peak_lineEdit_returnPressed
//!改变peak增益的槽函数
void MainWindow::on_gain_peak_lineEdit_returnPressed()
{
    float gain = ui->gain_peak_lineEdit->text().toFloat();
    dealMsg_obj->gainImage_peak = gain;
}

//!
//! \brief MainWindow::on_kalman_checkBox_clicked
//!开启或者关闭 kalman 滤波的槽函数
void MainWindow::on_kalman_checkBox_clicked()
{
    if(ui->kalman_checkBox->isChecked())
    {
        dealMsg_obj->isKalman = true;
    }else
    {
        dealMsg_obj->isKalman = false;
    }
}

//!
//! \brief MainWindow::on_pileUp_checkBox_clicked
//!
void MainWindow::on_pileUp_checkBox_clicked()
{
    bool flag = ui->pileUp_checkBox->isChecked();
    dealMsg_obj->is_pileUp_flag = flag;
}


//!
//! \brief MainWindow::on_filter_radioButton_clicked
//!是否进行滤波的槽函数
void MainWindow::on_filter_radioButton_clicked()
{
    bool isChecked = ui->filter_radioButton->isChecked();
    emit isFilter_signal(isChecked);
}

//!
//! \brief MainWindow::on_statistic_action_triggered
//!统计信息 弹出界面函数
void MainWindow::on_statistic_action_triggered()
{
    QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("该功能需要管理员权限，否则可能会异常退出"));
    statisticsDia_.setModal(true);
    statisticsDia_.show();
}

//!
//! \brief MainWindow::on_saveFile_action_triggered
//!文件保存界面 弹出
void MainWindow::on_saveFile_action_triggered()
{
    fileSave_dia.setModal(true);
    fileSave_dia.show();
}

//!
//! \brief isSaveFlagSlot
//! 文件保存 接收的槽函数
void MainWindow::isSaveFlagSlot(bool flag,QString filePath,int fileFormat)
{


    isSaveFlag = flag;        //是否进行存储
    saveFilePath = filePath;   //保存的路径  E:/..../.../的形式
    saveFileIndex = 1;      //文件标号；1作为开始
}

//!
//! \brief MainWindow::on_AutoCalibration_action_triggered
//! 自动校准的窗口
void MainWindow::on_AutoCalibration_action_triggered()
{
    autoCalibration_dia.setModal(true);
    autoCalibration_dia.show();
}


void MainWindow::on_autoCalibration_action_triggered()
{
    autoCalibration_dia.setModal(true);
    autoCalibration_dia.show();
}

//旋转角度设置的槽函数
void MainWindow::on_rotate_horizontalSlider_sliderMoved(int position)
{
    ui->widget->rotateRate = position;
    save3DSettingFile();
}

//缩放比例设置的槽函数
void MainWindow::on_scale_horizontalSlider_sliderMoved(int position)
{
    ui->widget->scaleRate = position;
    save3DSettingFile();
}

//拖放比例的槽函数
void MainWindow::on_translate_horizontalSlider_sliderMoved(int position)
{
    ui->widget->translateRate = 110 - position;
    save3DSettingFile();
}

//鼠标控制相关的配置 会保存到配置文件当中
void MainWindow::save3DSettingFile()
{
    QSettings configSetting("setting.ini", QSettings::IniFormat);

    int rotateRate = ui->widget->rotateRate;
    int scaleRate = ui->widget->scaleRate;
    int translateRate = 110-ui->widget->translateRate;

    configSetting.setValue("operation/Rotate_rate",rotateRate);
    configSetting.setValue("operation/Scale_rate",scaleRate);
    configSetting.setValue("operation/transtate_rate",translateRate);

}

//显示  peak的阈值
void MainWindow::on_peakOffset_lineEdit_returnPressed()
{
    float peakOffset = ui->peakOffset_lineEdit->text().toFloat();
    dealMsg_obj->peakOffset = peakOffset;
    qDebug()<<"dealMsg_obj->peakOffset = "<<dealMsg_obj->peakOffset;

}
//显示  平均的帧数
void MainWindow::on_averageNum_lineEdit_returnPressed()
{
    int average_frameNum = ui->averageNum_lineEdit->text().toInt();
    dealMsg_obj->averageNum = average_frameNum;
}
//显示 只显示中心区域
void MainWindow::on_centerShowYes_radioButton_clicked()
{

    dealMsg_obj->meanFilter_flag = true;
}
//显示 不显示中心区域
void MainWindow::on_centerShowNo_radioButton_clicked()
{

    dealMsg_obj->meanFilter_flag = false;
}


//!
//! \brief MainWindow::on_front_toolButton_clicked
//!设置正视图的视角
void MainWindow::on_front_toolButton_clicked()
{
    ui->widget->frontView_slot();
}

//!
//! \brief MainWindow::on_side_toolButton_clicked
//!侧视图视角
void MainWindow::on_side_toolButton_clicked()
{
    ui->widget->endView_slot();
}

//!
//! \brief MainWindow::on_down_toolButton_clicked
//!俯视图视角
void MainWindow::on_down_toolButton_clicked()
{
    ui->widget->verticalView_slot();
}


//!
//! \brief rece_oneFrame_slot
//!接收到一帧 统计数目加1
void MainWindow::rece_oneFrame_slot()
{
    frameCount++;
}

//!
//! \brief MainWindow::oneSec_timer_slot
//!显示帧率的槽函数
void MainWindow::oneSec_timer_slot()
{
    //    qDebug()<<"fps = "<<frameCount;

    QString fpsStr = "fps:"+QString::number(frameCount);
    fpsLabel.setText(fpsStr);
    frameCount = 0;

    //显示温度相关
    temperatureIndex++;
    if(5 == temperatureIndex)
    {
        temperatureIndex = 0;
        int iRet = 0;
        QString i2cAddr = "92";
        UINT uAddr = i2cAddr.toInt(NULL,16);
        UINT uReg = 0;
        USHORT uValue;
        QString sValue;

        iRet = ReadSensorReg(uAddr,uReg,&uValue,4,m_nDevID);
        if(iRet != DT_ERROR_OK)
        {
            return;
        }
        sValue = QString("%1").arg(uValue,4,16,QChar('0')).toUpper();
        int tmpValue = sValue.mid(0,2).toInt(NULL,16) *16 + sValue.mid(2,1).toInt(NULL,16);
        float temperture_f = tmpValue*0.0625;
        QString temperture_str = QString::number(temperture_f,'f',1) +QStringLiteral("℃");

        temptureLabel.setText("Temp:" + temperture_str);

//        QString str_log = "[Read I2C]:I2C_addr="+QString("%1").arg(uAddr,2,16,QChar('0')).toUpper() + ",Reg="+QString("%1").arg(uReg,2,16,QChar('0')).toUpper() + ",Value="+QString("%1").arg(uValue,4,16,QChar('0')).toUpper();
//        Display_log_slot(str_log);
    }

}


































/********************************   UV910 相关 ************************************************/

//!
//! \brief MainWindow::enable_UV910_trueOrFalse
//! \param flag
//! 使能控件信息
void MainWindow::enable_UV910_trueOrFalse(bool flag)
{
    ui->device_comboBox->setEnabled(flag);
    ui->EnumDevice_pushButton->setEnabled(flag);
    ui->initFilePath_lineEdit->setEnabled(flag);
    ui->load_iniFile_pushButton->setEnabled(flag);
}



//!
//! \brief MainWindow::ini_UV910
//!UV 910 的初始化
void MainWindow::init_UV910()
{
    CurrentSensor.ParaList=new unsigned short[8192*4];
    m_RunMode=RUNMODE_STOP;
    m_nDevID=-1;
    for(int i=0;i<8;i++)
    {
        pDeviceName[i]=nullptr;
    }
    m_isTV=false;
    EnumerateDothinkeyDevice();
}

//!
//! \brief MainWindow::on_load_iniFile_pushButton_clicked
//!加载配置文件
void MainWindow::on_load_iniFile_pushButton_clicked()
{
    QString fileName=QFileDialog::getOpenFileName();
    qDebug() << "filename = "<<fileName;

    if(fileName.isEmpty())
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("路径不能为空！"));
        return;
    }



    fileName = fileName.replace("\\","/");
    ui->initFilePath_lineEdit->setText(fileName);
    QSettings configSetting("setting.ini", QSettings::IniFormat);
    configSetting.setValue("init/filepath",fileName);

}


//!
//! \brief MainWindow::Sleepms
//! \param milsecond
//!睡眠时间
void MainWindow::Sleepms(double milsecond)
{
    LARGE_INTEGER freq;
    LARGE_INTEGER start_t,stop_t;
    double exe_time=0;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start_t);
    while(exe_time<milsecond)
    {
        QueryPerformanceCounter(&stop_t);
        exe_time=1e3*(stop_t.QuadPart-start_t.QuadPart)/freq.QuadPart;//1e3=1000
    }
}
bool MainWindow::OpenCamera()
{
    if(!bOpenDevice())
    {
        qDebug()<<"OpenDevice is fail";
        return false;
    }
    //close sensor
    SensorEnable(0,FALSE,m_nDevID);
    //close IO
    SetSoftPinPullUp(IO_NOPULL, m_nDevID);
    //close mclk
    qDebug()<<"*****************  mclk = "<<CurrentSensor.mclk;
    //    if (SetSensorClock(FALSE,(USHORT)(CurrentSensor.mclk*10), m_nDevID) != DT_ERROR_OK)
    if (SetSensorClock(FALSE,(USHORT)(80), m_nDevID) != DT_ERROR_OK)
    {
        CloseDevice(m_nDevID);
        qDebug()<<"SetSensorClock1";
        return false;
    }
    //Open Power/IO/Mclk
    {
        SENSOR_POWER Power[10] = {POWER_AVDD, POWER_DOVDD, POWER_DVDD, POWER_AFVCC, POWER_VPP};
        int Volt[10] = {0};
        int Current[10] = {300, 300, 300, 300, 100};//300mA
        BOOL Off[10] = {FALSE, FALSE, FALSE, FALSE, FALSE};
        BOOL OnOff[10] = {TRUE, TRUE, TRUE, TRUE, TRUE};
        CURRENT_RANGE range[5] = {CURRENT_RANGE_MA, CURRENT_RANGE_MA, CURRENT_RANGE_MA, CURRENT_RANGE_MA, CURRENT_RANGE_MA};
        //set all power  to 0, close the power firstly, voltage set to zero....
        if (PmuSetVoltage(Power, Volt,5, m_nDevID) != DT_ERROR_OK)
        {
            CloseDevice(m_nDevID);
            qDebug()<<"PmuSetVoltage";
            return false;
        }
        //wait for the power is all to zero....
        Sleep(50);
        //set PmuSetOnOff is On...
        if (PmuSetOnOff(Power, OnOff, 5, m_nDevID) != DT_ERROR_OK)
        {
            CloseDevice(m_nDevID);
            qDebug()<<"PmuSetOnOff";
            return false;
        }
        //wait for the PmuSetOnOff is On....
        Sleepms(50);
        // 1, power the dovdd...
        Volt[POWER_DOVDD] = 3300;//(int)(CurrentSensor.dovdd); // 1.8V
        PmuSetVoltage(Power, Volt, 5, m_nDevID);
        Sleepms(2);

        // 3, power on the  dvdd
        Volt[POWER_DVDD] = 1500;//(int)(CurrentSensor.dvdd);// 1.2V
        PmuSetVoltage(Power, Volt, 5, m_nDevID);
        Sleepms(2);

        // 2. power the avdd.
        Volt[POWER_AVDD] = 2800;//(int)(CurrentSensor.avdd); // 2.8V
        PmuSetVoltage(Power, Volt,5, m_nDevID);
        Sleepms(2);

        //4. power the afvcc ...
        Volt[POWER_AFVCC] = 2800;//(int)(afvcc); // 2.8V
        PmuSetVoltage(Power, Volt, 5, m_nDevID);
        Sleepms(2);
        //5. power the vpp...
        Volt[POWER_VPP] = 0; //(int)(vpp);
        PmuSetVoltage(Power, Volt, 5, m_nDevID);
        //设置时钟输入 switch to on...
        if (SetSensorClock(TRUE,80, m_nDevID) != DT_ERROR_OK)   //(USHORT)(CurrentSensor.mclk*10)
        {
            CloseDevice(m_nDevID);
            qDebug()<<"SetSensorClock";
            return false;
        }
        Sleepms(2);
        // set pin definition...
        {
            BYTE  pinDef[40] = {0};
            //mipi or hispi....
            if(CurrentSensor.port == PORT_MIPI || CurrentSensor.port == PORT_HISPI)
            {
                pinDef[0] = PIN_NC;
                pinDef[1] = PIN_D0;
                pinDef[2] = PIN_D2;//2;
                pinDef[3] = PIN_D1;
                pinDef[4] = PIN_D3;//3;
                pinDef[5] = PIN_D4;
                pinDef[6] = PIN_D5;
                pinDef[7] = PIN_D6;
                pinDef[8] = PIN_D7;
                pinDef[9] = PIN_D8;
                pinDef[10] = PIN_D9;
                pinDef[11] = PIN_NC;
                pinDef[12] = PIN_PCLK;
                pinDef[13] = PIN_HSYNC;
                pinDef[14] = PIN_VSYNC;
                pinDef[15] = PIN_NC;
                pinDef[16] = PIN_NC;
                pinDef[17] = PIN_MCLK;
                pinDef[18] = PIN_PWDN;
                pinDef[19] = PIN_RESET;
                pinDef[20] = PIN_SCL;
                pinDef[21] = PIN_SDA;
#if 1
                pinDef[22] = PIN_GPIO2;
                pinDef[23] = PIN_GPIO1;
                pinDef[24] = PIN_GPIO3;
                pinDef[25] = PIN_GPIO4;
#endif
#if 0
                pinDef[22] = 20;
                pinDef[23] = 16;
                pinDef[24] = 20;
                pinDef[25] = 20;
#endif
#if 0
                pinDef[22] = PIN_SPI_SCK;
                pinDef[23] = PIN_SPI_CS;
                pinDef[24] = PIN_SPI_SDO;//PIN_SPI_SDO;
                pinDef[25] = PIN_SPI_SDI;//PIN_SPI_SDI;
#endif

            }
            else if (CurrentSensor.port==0x81||CurrentSensor.port==0x83)
            {
                pinDef[0] = 20;
                pinDef[1] = 0;
                pinDef[2] = 2;
                pinDef[3] = 1;
                pinDef[4] = 3;
                pinDef[5] = 4;
                pinDef[6] = 5;
                pinDef[7] = 6;
                pinDef[8] = 7;
                pinDef[9] = 8;
                pinDef[10] = 9;
                pinDef[11] = 20;
                pinDef[12] = 10;
                pinDef[13] = 11;
                pinDef[14] = 12;
                pinDef[15] = 20;
                pinDef[16] = 20;
                pinDef[17] = 13;
                pinDef[18] = 15;
                pinDef[19] = 14;
                pinDef[20] = PIN_SPI_SCK; //PIN_SCL;
                pinDef[21] = PIN_SPI_SDO;//PIN_SDA ;
                pinDef[22] = PIN_SPI_CS;
                pinDef[23] = PIN_SPI_SDI;
                pinDef[24] = PIN_GPIO3;
                pinDef[25] = PIN_GPIO4;
            }
            else  //standard parallel..
            {
                //20140317 closed .这个是标准的。。。
#if 1
                pinDef[0]=PIN_GPIO1;//20;
                pinDef[1]=0;
                pinDef[2]=2;
                pinDef[3]=1;
                pinDef[4]=3;
                pinDef[5]=4;
                pinDef[6]=5;
                pinDef[7]=6;
                pinDef[8]=7;
                pinDef[9]=8;
                pinDef[10]=9;
                pinDef[11]=20;
                pinDef[12]=10;
                pinDef[13]=11;
                pinDef[14]=12;
                pinDef[15]=20;
                pinDef[16]=20;
                pinDef[17]=20;
                pinDef[18]=20;
                pinDef[19]=20;
                pinDef[20]=13;
                pinDef[21]=PIN_GPIO2;//20;
                pinDef[22]=14;
                pinDef[23]=15;
                pinDef[24]=18;
                pinDef[25]=19;
#endif
            }
            //配置柔性接口
            SetSoftPin(pinDef,m_nDevID);
        }
        //使能柔性接口
        EnableSoftPin(TRUE,m_nDevID);
        EnableGpio(TRUE,m_nDevID);
        //设置采集速率
        int SampleSpeed[5]={150,150,150,150,150};
        PmuSetSampleSpeed(Power,SampleSpeed,5,m_nDevID);
        Sleepms(100);
        // 设置量程
        PmuSetCurrentRange(Power,range,5,m_nDevID);
        //设置限流
        PmuSetOcpCurrentLimit(Power,Current,5,m_nDevID);
        //开启IO上拉电阻
        SetSoftPinPullUp(IO_PULLUP, m_nDevID);
    }
    Sleepms(50);
    //i2C init....
    SetSensorI2cRate(I2C_400K, m_nDevID);//设置IIC速率
    SetSensorI2cRapid(0, m_nDevID);//设置推挽模式
    SetI2CInterval(0, m_nDevID);//I2C byte to byte delay
    SetSensorI2cAckWait(100,m_nDevID);//ACK等待
    SetMipiImageVC(0,TRUE,1,m_nDevID);//设置VC通道
    Sleepms(50);

    //check sensor is on line or not ,if on line,init sensor to work....

    BYTE Pwdn2=0;
    BYTE Pwdn1=0;
    BYTE Reset=0;

    SensorEnable(CurrentSensor.pin^RESET_H, TRUE, m_nDevID);
    Sleepms(50);
    SensorEnable(CurrentSensor.pin, TRUE, m_nDevID);
    Sleepms(50);

    Pwdn2 = CurrentSensor.pin & PWDN_H ?  PWDN2_L : PWDN2_H;   //pwdn2 neg to pwdn1
    Pwdn1 = CurrentSensor.pin & PWDN_H ?  PWDN_H : PWDN_L;     //pwdn1
    Reset = CurrentSensor.pin & RESET_H ?  RESET_H : RESET_L;  //reset

    CurrentSensor.pin = Pwdn2 | Pwdn1 | Reset;
    SensorEnable(CurrentSensor.pin, 1, m_nDevID); //reset
#if 0
    BYTE pin1;
    pin1 = CurrentSensor.pin & 0x01 ? 0x00 : 0x04;
    pin1 |= CurrentSensor.pin & 0x03;

    SensorEnable(pin1 ^ 0x02, 1,m_nDevID); //reset
    SensorEnable(pin1 ^ 0x01, 1,m_nDevID); //reset
    SensorEnable(pin1, 1,m_nDevID);
#endif
    //check sensor is on line...
    /*if(SensorIsMe(&CurrentSensor, CHANNEL_A, 0,m_nDevID) != DT_ERROR_OK)
        {
            qDebug()<<"SensorIsMe";
            CloseCamera();
            return false;
        }*/

    MipiCtrlEx_t sMipiCtrlEx;
    GetMipiCtrlEx(&sMipiCtrlEx,m_nDevID);
    sMipiCtrlEx.byPhyType = 0;	//0是dphy，1是dphy deskew功能，2是cphy；
    sMipiCtrlEx.byLaneCnt = 4;  //lane个数设置，D-Phy支持1,2,4lane，C-Phy支持设置1,2,3trio；
    // 设置MIPI控制器
    SetMipiCtrlEx(&sMipiCtrlEx,m_nDevID);
    //init sensor....



    //    return true;
    qDebug()<<"slaveId="<<CurrentSensor.SlaveID<<"   ParaList="<<CurrentSensor.ParaList<<"  ParaListSize="<<CurrentSensor.ParaListSize<<"  CurrentSensor.mode"<<CurrentSensor.mode<<"  m_nDevID="<<m_nDevID;
    if(InitSensor(CurrentSensor.SlaveID,
                  CurrentSensor.ParaList,
                  CurrentSensor.ParaListSize,
                  CurrentSensor.mode,m_nDevID) != DT_ERROR_OK)
    {
        //        CloseCamera();
        //        return false;
        qDebug()<<"InitSensor function() error!";

    }


    m_isTV = false;
    USHORT TVBoard_Flag = 0;
    //check sensor is TV
    ReadSensorReg(0xba, 0x80, &TVBoard_Flag, I2CMODE_MICRON, m_nDevID);
    if(TVBoard_Flag == 0x5150)
    {
        m_isTV = 1;
    }
    //end check....
    if(CurrentSensor.type == D_YUV || CurrentSensor.type == D_YUV_SPI || CurrentSensor.type == D_YUV_MTK_S)
        SetYUV422Format(CurrentSensor.outformat, m_nDevID);
    else
        SetRawFormat(CurrentSensor.outformat, m_nDevID);

    m_PreviewWidth = CurrentSensor.width;
    m_PreviewHeight = CurrentSensor.height;     //& 0xfffe;

    USHORT roi_x0 = CurrentSensor.width >> 2;
    USHORT roi_y0 = CurrentSensor.height >> 2;
    USHORT roi_hb = 0;
    USHORT roi_vb = 0;
    USHORT roi_hnum = 1;
    USHORT roi_vnum = 1;
    // USHORT fifo_div = 2;
    roi_x0 = 0;
    roi_y0 = 0;
    roi_hb = 0;
    roi_vb = 0;
    roi_hnum = 1;
    roi_vnum = 1;
    //一个像素还有根据图片格式来算的 RAW10 4个像素分5字节
    //初始化设备
    InitRoi(0,0, CurrentSensor.width, m_isTV ? CurrentSensor.height >> 1 :CurrentSensor.height, 0, 0, 1, 1, CurrentSensor.type, TRUE, m_nDevID);	//以像素为单位
    SetSensorPort(CurrentSensor.port, CurrentSensor.width, CurrentSensor.height, m_nDevID);
    //调整MIPI RX CLOCK相位
    SetMipiClkPhase(0,m_nDevID);
    SetMipiEnable(TRUE,m_nDevID);//LP or HS
    Sleepms(10);
    CalculateGrabSize(&m_GrabSize, m_nDevID);
#if 0
    // 配置buffer
    {
        FrameBufferConfig config;
        GetFrameBufferConfig(&config,m_nDevID);
        //获取当前设备的BUFFER最大缓存深度（字节）
        m_uBufferSize = config.uBufferSize;
        UpdateData(FALSE);
        config.uMode = m_iBufferMode;
        config.uUpLimit = m_uUpLimitSize;
        SetFrameBufferConfig(&config,m_nDevID);
    }
#endif
    //open video....
    OpenVideo(m_GrabSize,m_nDevID);

#ifndef _ONLY_GRAB_IMAGE_


    InitDisplay(NULL,CurrentSensor.width,CurrentSensor.height,CurrentSensor.type, CHANNEL_A, NULL, m_nDevID);
    InitIsp(CurrentSensor.width, CurrentSensor.height, CurrentSensor.type, CHANNEL_A , m_nDevID);
    SetGamma(100, m_nDevID);  //Gamma  :default 100 is no gamma change....
    SetContrast(100, m_nDevID); //Contrast :default 100 is no contrast change...
    SetSaturation(128, m_nDevID);//Saturation: default 128 is no saturation change...
    SetDigitalGain(1.0, 1.0, 1.0, m_nDevID); //AWB digital gian R G B
#endif
    //    unsigned long RetSize = 0;
    //    FrameInfo m_FrameInfo;
    //    int width=CurrentSensor.width;
    //    int height=CurrentSensor.height;
    //    unsigned char *m_bufferRaw=new unsigned char[width*height*3];
    //    int bRet = GrabFrame(m_bufferRaw, m_GrabSize,&RetSize,&m_FrameInfo,m_nDevID);

    //    /// @retval DT_ERROR_OK：采集一帧图像成功
    //    /// @retval DT_ERROR_FAILD：采集一帧图像失败，可能不是完整的一帧图像数据
    //    /// @retval DT_ERROR_TIME_OUT：采集超时
    //    /// @retval DT_ERROR_INTERNAL_ERROR：内部错误
    ////    if(bRet == DT_ERROR_FAILD)
    ////    {
    ////        qDebug()<<"DT_ERROR_FAILD：采集一帧图像失败，可能不是完整的一帧图像数据";
    ////    }else
    //    if(bRet == DT_ERROR_TIME_OUT)
    //    {
    //        qDebug()<<"DT_ERROR_TIME_OUT：采集超时";
    //    }else if(bRet == DT_ERROR_INTERNAL_ERROR)
    //    {
    //        qDebug()<<"DT_ERROR_INTERNAL_ERROR：内部错误";
    //    }

    //    qDebug()<<bRet;
    //    qDebug()<<RetSize;


    //    unsigned char *m_pDisplayBuffer=new unsigned char[width*height*3];
    //    ImageProcess(m_bufferRaw, m_pDisplayBuffer,m_FrameInfo.uWidth, m_FrameInfo.uHeight, &m_FrameInfo,m_nDevID);
    //    qDebug()<<CurrentSensor.width;
    //    qDebug()<<CurrentSensor.height;
    //    QImage image(m_pDisplayBuffer, CurrentSensor.width, CurrentSensor.height, QImage::Format_RGB888);
    //    QImage mirroredImage = image.mirrored(true, true);//图像翻转
    //    // image.convertToFormat(QImage::Format_RGB32);
    //    QPixmap pix = QPixmap::fromImage(mirroredImage);

    //    const char* path;
    //    path = "E:\\right.bmp";
    //    mirroredImage.save(path,0);//保存图片

    //    ui->label->setPixmap(pix);
    //    //ui->openGLWidget->setParent(pix);
    //    m_RunMode=RUNMODE_PLAY;
    //    delete m_pDisplayBuffer;
    //    delete m_bufferRaw;
    //    m_bufferRaw=nullptr;
    //    m_pDisplayBuffer=nullptr;
    //    emit GetPmuCurrent();


    oneSec_timer.start(1000);
    return true;
}


void MainWindow::on_newFrame_pushButton_clicked()
{
    unsigned long RetSize = 0;
    FrameInfo m_FrameInfo;
    int width=CurrentSensor.width;
    int height=CurrentSensor.height;
    unsigned char *m_bufferRaw=new unsigned char[width*height*3];
    int bRet = GrabFrame(m_bufferRaw, m_GrabSize,&RetSize,&m_FrameInfo,m_nDevID);
    qDebug()<<bRet;
    qDebug()<<RetSize;

    QByteArray array ;
    QByteArray ba((char*)m_bufferRaw, RetSize);
    //    qDebug()<<ba.left(300);


    QString sFilePath = "E:/000.txt";
    QString tenFilePath= "E:/11.txt";
    QFile file(sFilePath);
    QFile tenfile(tenFilePath);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    tenfile.open(QIODevice::WriteOnly|QIODevice::Text);

    QByteArray hex =  ba.toHex().toUpper();
    QString text = hex;
    int len = text.length();
    QString file_to_write;
    QString ten_to_write;
    for(int i=0; i<len; i+=2)
    {
        file_to_write.append(text.mid(i,2)).append(" ");

        int num = text.mid(i,2).toInt(NULL,16);
        ten_to_write.append(QString::number(num)).append(" ");
    }

    //    ba.toHex()
    qDebug()<<"--------------------test = "<<text.left(200);
    QTextStream out(&file);
    QTextStream out2(&tenfile);
    out<<file_to_write<<endl;
    out2<<ten_to_write<<endl;
    file.close();
    tenfile.close();





    //    unsigned char *m_pDisplayBuffer=new unsigned char[width*height*3];
    //    ImageProcess(m_bufferRaw, m_pDisplayBuffer,m_FrameInfo.uWidth, m_FrameInfo.uHeight, &m_FrameInfo,m_nDevID);
    //    qDebug()<<CurrentSensor.width;
    //    qDebug()<<CurrentSensor.height;
    //    QImage image(m_pDisplayBuffer, CurrentSensor.width, CurrentSensor.height, QImage::Format_RGB888);
    //    QImage mirroredImage = image.mirrored(true, true);//图像翻转
    //     image.convertToFormat(QImage::Format_RGB32);
    //    QPixmap pix = QPixmap::fromImage(mirroredImage);

    //    const char* path;
    //    path = "E:\\right.bmp";
    //    image.save(path,0);//保存图片

    //    ui->label_2->setPixmap(pix);
    //    //ui->openGLWidget->setParent(pix);
    //    m_RunMode=RUNMODE_PLAY;
    //    delete m_pDisplayBuffer;
    //    delete m_bufferRaw;
    //    m_bufferRaw=nullptr;
    //    m_pDisplayBuffer=nullptr;

}




bool MainWindow::CloseCamera()
{
    m_RunMode=RUNMODE_STOP;
    //for power down
    CloseVideo(m_nDevID);
    ResetSensorI2cBus(m_nDevID);
    SensorEnable(CurrentSensor.pin^0x03,1,m_nDevID);//取反reset
    //SensorEnable(CurrentSensor.pin^0x01,1,m_nDevID);//取反pwdn
    Sleep(50);
    SetSensorClock(0,24*10,m_nDevID);
    SENSOR_POWER Power[10] = {POWER_AVDD, POWER_DOVDD, POWER_DVDD, POWER_AFVCC, POWER_VPP};
    int Volt[10] = {0};
    BOOL OnOff[10] = {FALSE,TRUE,FALSE,FALSE,FALSE};
    CURRENT_RANGE range[5] = {CURRENT_RANGE_UA, CURRENT_RANGE_UA, CURRENT_RANGE_UA, CURRENT_RANGE_UA, CURRENT_RANGE_UA};
#if 1
    //Test Standby
    {
        PmuSetCurrentRange(Power,range,5,m_nDevID);
        Sleep(1000);
        GetADValue();
    }
#endif
    SensorEnable(CurrentSensor.pin,FALSE,m_nDevID);
    SetSoftPinPullUp(IO_NOPULL,m_nDevID);
    /*使能UV910柔性接口不执行这两个函数不然IO口会上拉*/
    EnableGpio(FALSE,m_nDevID);
    EnableSoftPin(FALSE,m_nDevID);
    //设置5路电压值
    PmuSetVoltage(Power,Volt,5,m_nDevID);
    Sleep(50);
    PmuSetOnOff(Power,OnOff,5,m_nDevID);
    return true;
}

void MainWindow::EnumDevice()
{
    QMessageBox::information(this, "warning", "showimage", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(m_RunMode == RUNMODE_STOP)
    {
        EnumerateDothinkeyDevice();
    }
}
bool MainWindow::bOpenDevice()
{
    if(ui->device_comboBox->count()<= 0)
    {
        EnumerateDothinkeyDevice();
        if(ui->device_comboBox->count() <= 0)
        {
            return false;
        }
    }

    if(m_nDevID != ui->device_comboBox->currentIndex())
    {
        if (m_nDevID >= 0)
        {
            if(DT_ERROR_OK == IsDevConnect(m_nDevID))  //if old device is online,close it...
            {
                CloseDevice(m_nDevID);
            }
        }
    }
    else if (m_nDevID >= 0)
    {
        if(DT_ERROR_OK == IsDevConnect(m_nDevID))  //if current device is online ,returan directly...
        {
            // 			BYTE  TempDeviceSN[512];
            // 			int RetLen=0;
            // 			GetDeviceSN(TempDeviceSN,512,&RetLen,m_nDevID);
            // 			CString Tempstr2(TempDeviceSN);
            // 			AfxMessageBox(Tempstr2);
            return true;
        }
        else
        {
            CloseDevice(m_nDevID);
        }
    }
    int nCurDevID = ui->device_comboBox->currentIndex();
    int nRetID = 0;
    //打开当前选择的
    qDebug()<<pDeviceName[nCurDevID];
    if (OpenDevice(pDeviceName[nCurDevID], &nRetID, nCurDevID) != DT_ERROR_OK)
    {
        CloseDevice(nCurDevID);
        //AfxMessageBox("Open Device failed...");
        return false;
    }
    else
    {
        unsigned char  TempDeviceSN[512];
        int RetLen=0;
        GetDeviceSN(TempDeviceSN,512,&RetLen,nRetID);
        QByteArray ba((char*)TempDeviceSN, 512);
        QString str(ba);
        qDebug()<<str;
    }
    m_nDevID = nRetID;
#if 1
    DWORD Ver[4];
    int rect=GetFwVersion(0,Ver,m_nDevID);
    qDebug("BIN2 Version:%d.%d.%d.%d\n",Ver[0],Ver[1],Ver[2],Ver[3]);
    rect=GetFwVersion(1,Ver,m_nDevID);
    qDebug("BIN1 Version:%d.%d.%d.%d\n",Ver[0],Ver[1],Ver[2],Ver[3]);
    rect=GetFwVersion(2,Ver,m_nDevID);
    qDebug("BIN3 Version:%d.%d.%d.%d\n",Ver[0],Ver[1],Ver[2],Ver[3]);
    rect=GetFwVersion(3,Ver,m_nDevID);
    qDebug("BIN4 Version:%d.%d.%d.%d\n",Ver[0],Ver[1],Ver[2],Ver[3]);

#endif
    return true;
}

int MainWindow::EnumerateDothinkeyDevice()
{
    int i;
    int DevNum = 0;
    //  char* pDeviceName[8]={nullptr};

    int rect=EnumerateDevice(pDeviceName, 8, &DevNum);
    qDebug()<<"DevNum = "<<DevNum;
    if(!rect)
    {
        return 0;
    }
    //add to the device list....
    ui->device_comboBox->clear();
    if(DevNum)
    {
        for(i = 0; i < DevNum; i++)
        {
            QString str(pDeviceName[i]);
            //m_pDeviceName[i]=str;
            ui->device_comboBox->addItem(str);
        }
    }
    return DevNum;
}

void MainWindow::GetPmuCurrent()
{
    DWORD dwKitType = (GetKitType(m_nDevID)&0xFF);
    if(dwKitType  < 0x90)
        return ;
    SENSOR_POWER Power[5];
    int Current[5] = {0};

    Power[0] = POWER_AVDD;
    Power[1] = POWER_DOVDD;
    Power[2] = POWER_DVDD;
    Power[3] = POWER_AFVCC;
    Power[4] = POWER_VPP;
    PmuGetCurrent(Power, Current, 5, m_nDevID);
    qDebug("Iav:%.1f;Ido:%.1f;Idv:%.1f;Iaf:%.1f;Ivp:%.1f(mA)", Current[0]/1000000.0f, Current[1]/1000000.0f, Current[2]/1000000.0f, Current[3]/1000000.0f, Current[4]/1000000.0f);
}

void MainWindow::GetADValue()
{
    DWORD dwKitType = (GetKitType(m_nDevID)&0xFF);
    if(dwKitType  < 0x90)
        return ;

    SENSOR_POWER Power[5];
    int Current[5] = {0};

    Power[0] = POWER_AVDD;
    Power[1] = POWER_DOVDD;
    Power[2] = POWER_DVDD;
    Power[3] = POWER_AFVCC;
    Power[4] = POWER_VPP;
    PmuGetCurrent(Power, Current, 5, m_nDevID);

    if(m_RunMode == RUNMODE_STOP)
    {
        qDebug("Iav:%.1f;Ido:%.1f;Idv:%.1f;Iaf:%.1f;Ivp:%.1f(mA)", Current[0]/1000.0f, Current[1]/1000.0f, Current[2]/1000.0f, Current[3]/1000.0f, Current[4]/1000.0f);
    }
    else
    {
        qDebug("Iav:%.1f;Ido:%.1f;Idv:%.1f;Iaf:%.1f;Ivp:%.1f(mA)", Current[0]/1000000.0f, Current[1]/1000000.0f, Current[2]/1000000.0f, Current[3]/1000000.0f, Current[4]/1000000.0f);
    }
    //MIPI速度
    ULONG MipiFreq = 0;
    GetMipiClkFrequency(&MipiFreq,CHANNEL_A,m_nDevID);
    qDebug("%.1fMbps",(double)(MipiFreq*2/1000000));

}



void MainWindow::Load()
{
    //  QMessageBox::information(this, "warning", "showimage", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    //        QString fileName=QFileDialog::getOpenFileName();

    //    ui->initFilePath_lineEdit->setText(iniFile_path);
    //    QString fileName = "C:/Users/wenting.zhang/Desktop/FPGA.ini";

    QString fileName =  ui->initFilePath_lineEdit->text();
    qDebug() << "filename = "<<fileName;

    QSettings sensorIniFile(fileName, QSettings::IniFormat);
    QString tmpSensorName = sensorIniFile.value("Sensor/SensorName").toString();
    bool isChangeOK = false;
    char* ptr;
    QByteArray ba;
    ba = tmpSensorName.toLatin1();
    ptr = ba.data();
    memcpy(CurrentSensor.name,ptr,64);
    qDebug()<<"name"<<CurrentSensor.name;
    CurrentSensor.width = (UINT)sensorIniFile.value("Sensor/width").toInt(&isChangeOK);
    qDebug()<<"width"<<CurrentSensor.width;
    CurrentSensor.height = (UINT)sensorIniFile.value("Sensor/height").toInt(&isChangeOK);
    qDebug()<<"height"<<CurrentSensor.height;
    CurrentSensor.type = (UINT)sensorIniFile.value("Sensor/type").toInt(&isChangeOK);
    qDebug()<<"type"<<CurrentSensor.type;
    CurrentSensor.port = (UINT)sensorIniFile.value("Sensor/port").toInt(&isChangeOK);
    qDebug()<<"port"<<CurrentSensor.port;
    CurrentSensor.pin = (UINT)sensorIniFile.value("Sensor/pin").toInt(&isChangeOK);
    qDebug()<<"pin"<<CurrentSensor.pin;
    CurrentSensor.SlaveID = (UINT)sensorIniFile.value("Sensor/SlaveID").toString().toInt(&isChangeOK, 16);
    qDebug()<<"SlaveID"<<CurrentSensor.SlaveID;
    //    qDebug("SlaveID%0x",CurrentSensor.SlaveID);
    CurrentSensor.mode = (UINT)sensorIniFile.value("Sensor/mode").toInt(&isChangeOK);
    qDebug()<<"mode"<<CurrentSensor.mode;
    CurrentSensor.FlagReg = (UINT)sensorIniFile.value("Sensor/FlagReg").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagReg"<<CurrentSensor.FlagReg;
    CurrentSensor.FlagMask = (UINT)sensorIniFile.value("Sensor/FlagMask").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagMask"<<CurrentSensor.FlagMask;
    CurrentSensor.FlagData = (UINT)sensorIniFile.value("Sensor/FlagData").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagData"<<CurrentSensor.FlagData;
    CurrentSensor.FlagReg1 = (UINT)sensorIniFile.value("Sensor/FlagReg1").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagReg1"<<CurrentSensor.FlagReg1;
    CurrentSensor.FlagMask1 = (UINT)sensorIniFile.value("Sensor/FlagMask1").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagMask1"<<CurrentSensor.FlagMask1;
    CurrentSensor.FlagData1 = (UINT)sensorIniFile.value("Sensor/FlagData1").toString().toInt(&isChangeOK, 16);
    qDebug()<<"FlagData1"<<CurrentSensor.FlagData1;
    CurrentSensor.outformat = (UINT)sensorIniFile.value("Sensor/outformat").toInt(&isChangeOK);
    qDebug()<<"outformat"<<CurrentSensor.outformat;
    CurrentSensor.mclk = (UINT)sensorIniFile.value("Sensor/mclk").toInt(&isChangeOK);
    qDebug()<<"mclk"<<CurrentSensor.mclk;
    CurrentSensor.avdd = (UINT)sensorIniFile.value("Sensor/avdd").toInt(&isChangeOK);
    qDebug()<<"avdd"<<int(CurrentSensor.avdd);
    CurrentSensor.dovdd = (UINT)sensorIniFile.value("Sensor/dovdd").toInt(&isChangeOK);
    qDebug()<<"dovdd"<<int(CurrentSensor.dovdd);
    CurrentSensor.dvdd = (UINT)sensorIniFile.value("Sensor/dvdd").toInt(&isChangeOK);
    qDebug()<<"dvdd"<<int(CurrentSensor.dvdd);
    afvcc = (UINT)sensorIniFile.value("Sensor/afvcc").toInt(&isChangeOK);
    qDebug()<<"afvcc"<<afvcc;
    vpp = (UINT)sensorIniFile.value("Sensor/vpp").toInt(&isChangeOK);
    qDebug()<<"vpp"<<vpp;
    CurrentSensor.Ext0 = (UINT)sensorIniFile.value("Sensor/Ext0").toInt(&isChangeOK);
    CurrentSensor.Ext1 = (UINT)sensorIniFile.value("Sensor/Ext1").toInt(&isChangeOK);
    CurrentSensor.Ext2 = (UINT)sensorIniFile.value("Sensor/Ext2").toInt(&isChangeOK);
    //    bloadIniFile(fileName);
}


//!
//! \brief MainWindow::on_openDevice_pushButton_clicked
//! 1、加载路径中的配置文件
//! 2、初始化设备
void MainWindow::on_openDevice_pushButton_clicked()
{
    QString filePath = ui->initFilePath_lineEdit->text();


    QFileInfo fileInfo(filePath);
    if(!fileInfo.isFile())
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("文件路径错误，请重新选择!"));
        return ;
    }


    if(QStringLiteral("打开") == ui->openDevice_pushButton->text())
    {
        isReceUV910_flag = true;
        oneSec_timer.start(1000);
        emit start_openUV910_signal(filePath);
    }else
    {
        isReceUV910_flag = false;
        emit stop_UV910_signal();
        ui->openDevice_pushButton->setText(QStringLiteral("打开"));
        QString log_str = "[UV910 closed!]";
        Display_log_slot(log_str);
        enable_UV910_trueOrFalse(true);
    }


}

//!
//! \brief MainWindow::link_UV910_return_slot
//!  连接的返回信息槽函数
void  MainWindow::link_UV910_return_slot(bool flag)
{
    if(flag)   //说明连接成功
    {
        ui->openDevice_pushButton->setText(QStringLiteral("关闭"));
        QString logStr = "[Open UV910 success!]";
        Display_log_slot(logStr);
        enable_UV910_trueOrFalse(false);

    }else
    {
        QString logStr = "[Open UV910 failed!]";
        Display_log_slot(logStr);
        enable_UV910_trueOrFalse(true);
    }

}




////测试I2C的功能
//void MainWindow::on_pushButton_clicked()
//{
//    ULONG Freq;
//    QString str,saddr;
//    saddr="addr:";
//    //GetMipiClkFrequency(&Freq,CHANNEL_A,0);
//    //str.Format("%.1fMsps",(double)Freq*2/1000000);
//    UCHAR val;
//    int ret;
//    for(int i=0;i<0xff;)
//    {
//        ret=WriteSensorReg(i,0,0,0,m_nDevID);
//        if(ret==DT_ERROR_OK)
//        {
//            //          str.Format("0x%x,",i);
//            str = QString("0x%1").arg(i,4,16,QChar('0'));
//            saddr.append(str);
//        }
//        i=i+2;
//    }

//    qDebug()<<"addr = "<<saddr;
//}



//!
//! \brief MainWindow::on_I2C_read_pushButton_clicked
//!  读取i2c 的功能
void MainWindow::on_I2C_read_pushButton_clicked()
{
    int iRet = 0;
    UINT uAddr = ui->I2C_addr_lineEdit->text().toInt(NULL,16);
    UINT uReg = ui->I2C_Reg_lineEdit->text().toInt(NULL,16);
    USHORT uValue;
    QString sValue;

    iRet = ReadSensorReg(uAddr,uReg,&uValue,0,m_nDevID);
    if(iRet != DT_ERROR_OK)
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("读取I2C出错!"));
        return;
    }
    sValue = QString("%1").arg(uValue,2,16,QChar('0')).toUpper();
    ui->I2C_value_lineEdit->setText(sValue);

    QString str_log = "[Read I2C]:I2C_addr="+QString("%1").arg(uAddr,2,16,QChar('0')).toUpper() + ",Reg="+QString("%1").arg(uReg,2,16,QChar('0')).toUpper() + ",Value="+QString("%1").arg(uValue,2,16,QChar('0')).toUpper();
    Display_log_slot(str_log);
}

//!
//! \brief MainWindow::on_I2C_write_pushButton_clicked
//!  I2C写入功能
void MainWindow::on_I2C_write_pushButton_clicked()
{
    UINT uAddr = ui->I2C_addr_lineEdit->text().toInt(NULL,16);
    UINT uReg = ui->I2C_Reg_lineEdit->text().toInt(NULL,16);
    UINT uValue = ui->I2C_value_lineEdit->text().toInt(NULL,16);

    if (WriteSensorReg(uAddr, uReg, uValue, 0, m_nDevID) == FALSE)
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("写入I2C出错!"));
        return;
    }
    QString str_log = "[Write I2C]:I2C_addr="+QString("%1").arg(uAddr,2,16,QChar('0')).toUpper() + ",Reg="+QString("%1").arg(uReg,2,16,QChar('0')).toUpper() + ",Value="+QString("%1").arg(uValue,2,16,QChar('0')).toUpper();
    Display_log_slot(str_log);
}


//写入I2C的槽函数
void MainWindow::write_I2C_slot(QString addressStr,QString valueStr)
{

    qDebug()<<"write_I2C_slot: address = "<<addressStr<<"  valueStr="<<valueStr;
    UINT uAddr = ui->I2C_addr_lineEdit->text().toInt(NULL,16);
    UINT uReg = addressStr.toInt(NULL,16);
    UINT uValue = valueStr.toInt(NULL,16);

    if (WriteSensorReg(uAddr, uReg, uValue, 0, m_nDevID) == FALSE)
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("写入I2C出错!"));
        return;
    }
}


//!
//! \brief MainWindow::keyPressEvent
//! \param e
//! 按键
void MainWindow::keyPressEvent(QKeyEvent *e)
{
    //    qDebug()<<"key num = "<<e->key();
    if(90 == e->key())
    {
        ui->groupBox_5->setVisible(true);
    }else if(88 == e->key())
    {
        ui->groupBox_5->setVisible(false);
    }
}


//!
//! \brief MainWindow::on_about_action_triggered
//!关于   查看版本信息
void MainWindow::on_about_action_triggered()
{
    about_dia.show();
}









void MainWindow::on_cameraPara_action_triggered()
{
    cameraSetting_dia.show();
}
//!
//! \brief MainWindow::on_kalmanPara_lineEdit_returnPressed
//!卡尔曼系数
void MainWindow::on_kalmanPara_lineEdit_returnPressed()
{
    dealMsg_obj->kalmanOffset_para = ui->kalmanPara_lineEdit->text().toFloat();
    qDebug()<<"kalmanPara tof = "<<ui->kalmanPara_lineEdit->text().toFloat();
}

//!
//! \brief MainWindow::on_kalmanPara_peak_lineEdit_returnPressed
//! 卡尔曼滤波 的peak值系数
void MainWindow::on_kalmanPara_peak_lineEdit_returnPressed()
{
    dealMsg_obj->kalmanOffset_peak_para = ui->kalmanPara_peak_lineEdit->text().toFloat();
    qDebug()<<"kalmanPara  peak = "<<ui->kalmanPara_peak_lineEdit->text().toFloat();
}


//平均时候的阈值
void MainWindow::on_meanTof_offset_lineEdit_returnPressed()
{
    //    float meanTof_offset = ui->meanTof_offset_lineEdit->text().toFloat();
    //    dealMsg_obj->meanFilter_offset = meanTof_offset;
}

///*****************DCR测试暗计数相关***********************************/
//文件选择
void MainWindow::on_balckileSave_toolButton_clicked()
{
    QString file_path = QFileDialog::getExistingDirectory(this,QStringLiteral("请选择文件保存路径..."),"./");
    if(file_path.isEmpty())
    {
        qDebug()<<QStringLiteral("没有选择路径")<<endl;
        QMessageBox::information(NULL,QStringLiteral("告警"),QStringLiteral("保存路径不能为空"));
        return;
    }
    else
    {
        file_path.append("/");
        qDebug() << file_path << endl;
        ui->blackSavePath_lineEdit->setText(file_path);
    }
}

// 行和列的文件
void MainWindow::on_black_rowAndCol_toolButton_clicked()
{
    QString fileName=QFileDialog::getOpenFileName();
    qDebug() << "filename = "<<fileName;

    if(fileName.isEmpty())
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("路径不能为空！"));
        return;
    }
    ui->rowAndCol_lineEdit->setText(fileName);


    QString rowAndCol_str[1000];
    QFile readRowAndColFile(fileName);
    rowAndCol_strlist.clear();
    ui->rowAndCol_plainTextEdit->clear();
    QStringList tmpStrList;
    QString tmpStr;
    if (readRowAndColFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&readRowAndColFile);
        int i = 0;
        while (!in.atEnd())
        {
            tmpStr = in.readLine();


            if(!(tmpStr.isEmpty()))
            {
                tmpStrList = tmpStr.split(",");
                tmpStr = tmpStrList[0];
                rowAndCol_strlist.append(tmpStr);
                tmpStr = tmpStrList[1];
                rowAndCol_strlist.append(tmpStr);
            }


            i++;
        }
        readRowAndColFile.close();

    }else{
        QString log_str = "[load conf file error]";
        Display_log_slot(log_str);
    }





    QString showStr;

    qDebug()<<" len = "<<rowAndCol_strlist.length();
    for(int i=0; i<rowAndCol_strlist.length(); i+=2)
    {
        showStr = rowAndCol_strlist[i] +"      "+ rowAndCol_strlist[1+i];
        ui->rowAndCol_plainTextEdit->appendPlainText(showStr);
    }

}




//开始进行数据采取
void MainWindow::on_blackStart_pushButton_clicked()
{
    QString filePath = ui->blackSavePath_lineEdit->text();
    int savedFrameNum = ui->black_savedFrame_lineEdit->text().toInt();
    QString inteStr = ui->blackIntegrateTime_lineEdit->text();
    emit start_blackGetCount_signal(filePath,inteStr,savedFrameNum,rowAndCol_strlist);

    ui->blackStart_pushButton->setText(QStringLiteral("已开始采集"));
}

//返回DCR计数保存的帧数
void MainWindow::sendSavedFrame_slot(int frame)
{
    ui->blackFrame_label->setText(QString::number(frame));
}



/**********************RawData 數據相關*******************************/
//保存RowData信息界面
void MainWindow::on_RawData_action_triggered()
{
    rawData_dia.setModal(true);
    rawData_dia.show();
}


//開始接收rawData数据的槽函数
void MainWindow::on_start_rawDataSave_slot(QString array)
{
    isReceUV910_flag = true;
    emit receUV910_data_signal();       //数据接收线程开始接收数据
}


/********ROWdATA MA 相关***********/
void MainWindow::on_Hist_MA_action_triggered()
{
    Hist_MA_dia.setModal(true);
    Hist_MA_dia.show();
}

void MainWindow::startReceUV910_rowData_MA_slot(bool flag)
{
    isReceUV910_flag = flag;
    emit receUV910_data_signal();       //数据接收线程开始接收数据
    qDebug()<<"MainWindow::startReceUV910_rowData_MA_slot(bool flag)";
}


//!
//! \brief MainWindow::on_peakNormalization_radioButton_clicked
//!  是否显示归一化的peak值
void MainWindow::on_isShowNormalizationPeak_checkBox_clicked()
{

    qDebug()<<"is show normalzation peak";
    if(ui->isShowNormalizationPeak_checkBox->isChecked())
    {
        dealMsg_obj->isShowNormalizationOfPeak_flag = true;
    }else
    {
        dealMsg_obj->isShowNormalizationOfPeak_flag = false;
    }
}


void MainWindow::on_rawDatapushButton_clicked()
{
    dealMsg_obj->UV910_RawData_deal_slot("");
}



