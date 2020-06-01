#include "receuv910.h"

extern SensorTab CurrentSensor;
extern int m_RunMode;
extern int m_nDevID;
extern bool isReceUV910_flag;  //是否接收数据的标识

receUV910::receUV910(QObject *parent) : QObject(parent)
{

    qDebug()<<"receUV910 thread has start";
}
void receUV910::receUV910_data_slot()
{
    FrameInfo m_FrameInfo;
    unsigned long RetSize = 0;
    int width=CurrentSensor.width;
    int height=CurrentSensor.height;
    unsigned char *m_bufferRaw=new unsigned char[width*height*3];
    int bRet;
    int usedSize = 153600;


    while(isReceUV910_flag)
    {
        bRet = GrabFrame(m_bufferRaw, m_GrabSize,&RetSize,&m_FrameInfo,m_nDevID);

        if(bRet == DT_ERROR_OK)             //RetSize=614400       每行的数据量：2*640= 1280个字节     1280*120 = 153600
        {
//            qDebug()<<"size = "<<RetSize;
            if(1920000 == RetSize)   //此长度说明是RawData类型数据
            {
                QByteArray array((char*)m_bufferRaw, RetSize);
                if(1 == isRawDataSave_flag)   //保存rowData数据
                {
                    emit UV910_RawData_deal_signal(array);

                }else if(2 == isRawDataSave_flag)   //rowData MA 数据相关
                {
                    emit UV910_RawData_MA_deal_signal(array);
                }

            }else
            {
                QByteArray array((char*)m_bufferRaw, RetSize);
//                qDebug()<<"size ="<<RetSize<<" ,data ="<<array.toHex().left(100);
                emit UV910_Qtech_deal_signal(array);
            }


            emit rece_oneFrame_signal();

        }else if(bRet == DT_ERROR_TIME_OUT)  //：采集超时
        {
            qDebug()<<"DT_ERROR_TIME_OUT";
        }else if(bRet ==DT_ERROR_INTERNAL_ERROR )    //：内部错误
        {
            qDebug()<<"DT_ERROR_INTERNAL_ERROR";
        }
    }

}

//!
//! \brief receUV910::startReceUV910_rowData_slot
//! 开始接收RowData
void receUV910::startReceUV910_rowData_slot(bool flag)
{
    if(flag)
    {
        isRawDataSave_flag = 1;
    }else
    {
        isReceUV910_flag = false;
    }

    qDebug()<<"startReceUV910_rowData_slot(bool flag) = "<<flag;


}

//接收rowdataMA 的模式 isRawDataSave_flag = 2;
void receUV910::startReceUV910_rowData_MA_slot(bool flag)
{
    qDebug()<<"startReceUV910_rowData_MA_slot(bool flag) = ";
    if(flag)
    {
        isRawDataSave_flag = 2;
    }else
    {
        isReceUV910_flag = false;
    }
}



//!
//! \brief MainWindow::ini_UV910
//!UV 910 的初始化
void receUV910::init_UV910()
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
//! \brief MainWindow::Sleepms
//! \param milsecond
//!睡眠时间
void receUV910::Sleepms(double milsecond)
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
bool receUV910::OpenCamera()
{
    if(!bOpenDevice())
    {
        qDebug()<<"OpenDevice is fail";
        emit link_UV910_return_signal(false);
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
            emit link_UV910_return_signal(false);
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
        emit link_UV910_return_signal(false);

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

    emit link_UV910_return_signal(true);
    return true;
}


void receUV910::on_newFrame_pushButton_clicked()
{
}




bool receUV910::CloseCamera()
{

    qDebug()<<"close camera has emit";

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

void receUV910::EnumDevice()
{
    //    QMessageBox::information(this, "warning", "showimage", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(m_RunMode == RUNMODE_STOP)
    {
        EnumerateDothinkeyDevice();
    }
}


bool receUV910::bOpenDevice()
{

    int numDev  = EnumerateDothinkeyDevice();
    if(numDev <= 0)
    {
        qDebug()<<"can not find device!";
        return false;
    }


    if(m_nDevID != 0)
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
    //    int nCurDevID = ui->device_comboBox->currentIndex();
    int nCurDevID = 0;
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

int receUV910::EnumerateDothinkeyDevice()
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
    //    ui->device_comboBox->clear();
    if(DevNum)
    {
        for(i = 0; i < DevNum; i++)
        {
            QString str(pDeviceName[i]);
            //m_pDeviceName[i]=str;
            //            ui->device_comboBox->addItem(str);
            qDebug()<<"EnumerateDothinkeyDevice() =  "<<str;
        }
    }
    return DevNum;
}

void receUV910::GetPmuCurrent()
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

void receUV910::GetADValue()
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
void receUV910::Play()
{
    if(m_RunMode == RUNMODE_STOP)
    {
        // m_roi_state = 0;
        if(OpenCamera())
        {
            qDebug()<<"open camera success!";
        }
        else
        {
            //            QMessageBox::information(this, "warning", "Open Camera Fail!");
            qDebug()<<"Open Camera Fail!";
        }
    }
    else
    {
        CloseCamera();
        //        ui->PlayCamera->setText("play");
        qDebug()<<"close camera success!";
    }
}


void receUV910::Load(QString filePath)
{

//    QString fileName =  "C:/Users/wenting.zhang/Desktop/DtSample_200408/4310_DTK_0408.ini";
    QString fileName = filePath;

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
    bloadIniFile(fileName);
}


//!
//! \brief MainWindow::on_openDevice_pushButton_clicked
//! 1、加载路径中的配置文件
//! 2、初始化设备
void receUV910::on_openDevice_pushButton_clicked(QString filePath)
{
    Load(filePath);

    Sleepms(1000);
    if(OpenCamera())
    {
        qDebug()<<"on_openDevice_pushButton_clicked()  openCamera() sucess";
    }
    else
    {
        QString Disp_log = "[Open device failed!]";
        qDebug()<<Disp_log;
        CloseCamera();
        return;
    }



}

bool receUV910::bloadIniFile(QString sFile)
{
    QFile file(sFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 0;
    }
    QTextStream inData(&file);
    QString line = inData.readLine();
    bool isFindStrat = false;
    int tmpFind;
    QString sSection = QString::fromLocal8Bit("[ParaList]");
    sSection = sSection.toLower();
    sSection = sSection.trimmed();
    QString sVal = 0;
    QString sReg = 0;
    UINT dataLen = 0;
    unsigned short *pParaList=CurrentSensor.ParaList;;

    do
    {
        //再读一行
        line = inData.readLine();
        if (line.isNull())
        {
            break;
        }
        //处理一段
        tmpFind = line.indexOf(QString::fromLocal8Bit("//"));
        if (tmpFind == 0)
        {
            continue;
        }
        else if (tmpFind > 0)
        {
            line = line.left(tmpFind);
        }

        tmpFind = line.indexOf(QString::fromLocal8Bit("]"));
        if (tmpFind == 0)
        {
            continue;
        }
        else if (tmpFind > 0)
        {
            line = line.left(tmpFind + 1);
        }
        line = line.toLower();
        line = line.trimmed();

        if (!isFindStrat)                              //not find
        {
            if (QString::compare(line, sSection) == 0)
            {
                isFindStrat = true;
                continue;
            }
            continue;
        }

        if (line.isEmpty())
            continue;
        else if (line.left(1) == QString::fromLocal8Bit(","))
            continue;
        else if (line.left(1) == QString::fromLocal8Bit(";"))
            continue;
        else if (line.left(1) == QString::fromLocal8Bit("/"))
            continue;

        else if (line.left(1) == QString::fromLocal8Bit("["))        //遇到新的列表
        {
            break;
        }
        //以逗号分开
        int len = line.size();
        tmpFind = line.indexOf(QString::fromLocal8Bit(","));
        if (tmpFind <= 0 || tmpFind > len)
        {
            continue;
        }
        sReg = line.left(tmpFind);
        sVal = line.right(len - tmpFind - 1);
        tmpFind = sVal.indexOf(QString::fromLocal8Bit(","));
        if (tmpFind > 0)
        {
            sVal = sVal.left(tmpFind);
        }

        bool isChangeOK = false;
        ushort tmpReg = sReg.toInt(&isChangeOK, 16);
        if (!isChangeOK)
        {
            continue;
        }
        ushort tmpVal = sVal.toInt(&isChangeOK, 16);
        if (!isChangeOK)
        {
            continue;
        }
        dataLen += 2;
        *(pParaList+CurrentSensor.ParaListSize) = tmpReg;
        *(pParaList+CurrentSensor.ParaListSize+1) = tmpVal;
        CurrentSensor.ParaListSize += 2;
    } while (true);


    qDebug()<<"strat";
    for(int i=0;i<CurrentSensor.ParaListSize;i+=2)
    {
        qDebug("0x%04x,0x%04x",CurrentSensor.ParaList[i],CurrentSensor.ParaList[i+1]);
    }
    qDebug()<<"end";
    CurrentSensor.ParaListSize=dataLen;
    qDebug("%d",dataLen);

    return true;
}


