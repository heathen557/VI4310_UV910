#include "dealusb_msg.h"
#define miu_meter 1e-3

//图像显示相关
extern QMutex mutex_3D;  //3D点云/2D传输的互斥锁
extern QImage tofImage;
extern QImage intensityImage;
extern pcl::PointCloud<pcl::PointXYZRGB> pointCloudRgb;
extern bool isShowPointCloud;  //是否有点云数据 ，有的话显示否则不显示

////鼠标点击显示时相关
extern QMutex mouseShowMutex;
extern float mouseShowTOF[160][120];
extern float mouseShowPEAK[160][120];
extern float mouseShowDepth[160][120];
extern float mouseShow_X[160][120];
extern float mouseShow_Z[160][120];

///*保存用到的标识*/
extern bool isSaveFlag;        //是否进行存储
extern QString saveFilePath;   //保存的路径  E:/..../.../的形式
extern int saveFileIndex;      //文件标号；1作为开始



///*******统计信息相关的变量***********/
extern QMutex statisticMutex;
extern vector<vector<int>> allStatisticTofPoints;   //用于统计 均值和方差的 容器  TOF
extern vector<vector<int>> allStatisticPeakPoints;   //用于统计 均值和方差的 容器  TOF

///***rawData 数据保存相关**/
extern QString rawData_savePath;    //保存的路径
extern int exposure_num;            //曝光次数
extern int rawData_saveFrameNums;   //保存帧数
//extern bool isReceRawData_flag;     //是否进行保存rowData数据
bool isReceRawData_flag;



DealUsb_msg::DealUsb_msg(QObject *parent) : QObject(parent),
    microQimage(160,120, QImage::Format_RGB32),macroQimage(160,120, QImage::Format_RGB32)
{

    pointCloudRgb.width = 16384;
    pointCloudRgb.height = 1;
    pointCloudRgb.resize(pointCloudRgb.width);
    isKalman = true;
    meanFilter_flag = false;   //是否进行点云显示噪点的过滤
    meanFilter_offset = 50;     //点云平均阈值时候的噪点去除阈值

    isShowNormalizationOfPeak_flag = false;

    tempRgbCloud.width = 19200;
    tempRgbCloud.height = 1 ;
    tempRgbCloud.points.resize(tempRgbCloud.width);

    LSB = 0.031; //时钟频率
    isFilterFlag = false ;    //初始化时不进行滤波
    lineSelect = false;       //初始化 不切换两行像素
    isTOF = true;
    localFile_timer = NULL;
    gainImage = 1;

    gainImage_tof = 1;        //增益
    gainImage_peak = 1;
    haveIndex = 0;
    peakOffset = 0;   //设置为阈值，小于这个值的认为是无效数据，将接收到的tof值设置为0  ::此功能预留，面阵_1028效果较好，但是对其他数据会滤掉大部分有效数据
    isOnlyCenterShow_flag = false;   //是否只显示中心区域的标识，设置为true则只显示中心光较强的区域（超过范围的点xyz坐标全部设置为0），设置为false则显示全部点云数据；默认false;
    averageNum = 1;            //滑动平均的帧数 , 默认为1

    /********pileUp 以及自动校准相关的参数***************/
    is_pileUp_flag = true;
    isAutoCalibration_flag = false;  //默认不使用
    ishave_Four = 0 ;               // 初始设置为0，逐渐+1 ==4 时进行判断
    calibration_mean_num = 100;
    radius = 0.2;

    camera_dis = 23;
    f = 6;

    //初始化相机参数
    QSettings configSetting("setting.ini", QSettings::IniFormat);
    f = configSetting.value("camera/focal_length").toFloat();
    camera_dis = configSetting.value("camera/camera_diff").toFloat();
    integrateNum_ = configSetting.value("camera/integrate_num").toInt();

    qDebug()<<"dealUsbMSG ,     f="<<f<<"   camera_dis="<<camera_dis<<"  integrateNum_="<<integrateNum_;

    //总共有16384个点，针对每一个点开启一个独立的容器进行存储相关内容    统计相关
    statisticStartFlag = true;
    statisticFrameNumber = 10;
    vector<int> singlePoint;
    for(int i=0; i<19200; i++)
    {
        tempStatisticTofPoints.push_back(singlePoint);
        tempStatisticPeakPoints.push_back(singlePoint);
        allStatisticTofPoints.push_back(singlePoint);
        allStatisticPeakPoints.push_back(singlePoint);

        //kalman filters  卡尔曼滤波算法
        x_k[i] = 0;
        p_k[i] = 0;
        K[i] = 0;

        peak_x_k[i] = 0;
        peak_p_k[i] = 0;
        peak_K[i] = 0;
    }
    kalman_F = 1;
    kalman_H = 1;
    kalman_Q = 1;
    kalman_R = 5;

    peak_kalman_F =1;
    peak_kalman_H =1;
    peak_kalman_Q =1;
    peak_kalman_R = 5;

    kalmanOffset_para = 1;   //两帧之间offset的系数


    //初始化文件保存槽函数
    init_File_save();

    /**保存rawData数据相关*/
    isReceRawData_flag = false;
    currentRawDataFrame = 0;   //rowData的帧数
}

////初始化文件保存的槽函数
void DealUsb_msg::init_File_save()
{
    isSaveRawTof = true;
    isSavePileUpTof = false;
    isSaveFilterTof = false;
    isSaveRawPeak = true;
    isSaveFilterPeak = false;
    isSaveX = false;
    isSaveY = false;
    isSaveZ = false;

    tofPeakNum[0] = QStringLiteral("Rawtof,RawPeak\n");
}


//!\
//! 修改要保存的文件信息的槽函数
void DealUsb_msg::alter_fileSave_slot(bool saveRawTof,bool savePileUpTof,bool saveFilterTof,bool saveRawPeak,bool saveFilterPeak,bool saveX,bool saveY,bool saveZ)
{
    isSaveRawTof = saveRawTof;
    isSavePileUpTof = savePileUpTof;
    isSaveFilterTof = saveFilterTof;
    isSaveRawPeak = saveRawPeak;
    isSaveFilterPeak = saveFilterPeak;
    isSaveX = saveX;
    isSaveY = saveY;
    isSaveZ = saveZ;

    if(saveRawTof)
        tofPeakNum[0] = QStringLiteral("RawTof,");
    if(savePileUpTof)
        tofPeakNum[0].append(QStringLiteral("pileUpTof,"));
    if(saveFilterTof)
        tofPeakNum[0].append(QStringLiteral("FilterTof,"));
    if(saveRawPeak)
        tofPeakNum[0].append(QStringLiteral("RawPeak,"));
    if(saveFilterPeak)
        tofPeakNum[0].append(QStringLiteral("FilterPeak,"));
    if(saveX)
        tofPeakNum[0].append(QStringLiteral("X,"));
    if(saveY)
        tofPeakNum[0].append(QStringLiteral("Y(depth),"));
    if(saveZ)
        tofPeakNum[0].append(QStringLiteral("Z"));

    tofPeakNum[0].append("\n");


}


void DealUsb_msg::loadLocalArray()
{
    //加载数据的配置四个相关的配置矩阵的部分

    //加载tofOffsetArray.txt配置集
    QFile tofOffsetArray_file("tofOffsetArray.txt");
    QString tofOffsetArray_line[19210];
    QString log_str;
    if (tofOffsetArray_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&tofOffsetArray_file);
        int i = 0;
        while (!in.atEnd())
        {
            tofOffsetArray_line[i] = in.readLine();
            i++;
        }
        tofOffsetArray_file.close();

        if(i<19199)
        {
            log_str = "[load conf file error]:tofOffsetArray.txt";
            emit Display_log_signal(log_str);

        }else{
            log_str = "[load conf file success]:tofOffsetArray.txt";
            emit Display_log_signal(log_str);
        }
    }else{
        log_str = "[load conf file error]:tofOffsetArray.txt";
        emit Display_log_signal(log_str);
    }



    //加载xf_position.txt配置集
    QFile xf_position_file("xf_position.txt");
    QString xf_position_line[200];
    if (xf_position_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&xf_position_file);
        int i = 0;
        while (!in.atEnd())
        {
            xf_position_line[i] = in.readLine();
            i++;
        }
        xf_position_file.close();

        if(i<159)
        {
            log_str = "[load conf file error]:xf_position.txt";
            emit Display_log_signal(log_str);
        }else{
            log_str = "[load conf file success]:xf_position.txt";
            emit Display_log_signal(log_str);
        }
    }else{
        log_str = "[load conf file error]:xf_position.txt";
        emit Display_log_signal(log_str);
    }


    //加载yf_position.txt配置集
    QFile yf_position_file("yf_position.txt");
    QString yf_position_line[200];
    if (yf_position_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&yf_position_file);
        int i = 0;
        while (!in.atEnd())
        {
            yf_position_line[i] = in.readLine();
            i++;
        }
        yf_position_file.close();

        if(i<119)
        {
            log_str = "[load conf file error]:yf_position.txt";
            emit Display_log_signal(log_str);
        }else {
            log_str = "[load conf file success]:yf_position.txt";
            emit Display_log_signal(log_str);
        }
    }else{
        log_str = "[load conf file error]:yf_position.txt";
        emit Display_log_signal(log_str);
    }



    for(int i=0;i<19200;i++)
    {
        tofOffsetArray[i] = tofOffsetArray_line[i].toFloat();
    }

    for(int i=0; i<160; i++)
    {
        xf_position[i] = xf_position_line[i].toFloat();
        //        qDebug()<<"xf_position = "<<i<<"   = "<<xf_position[i];
    }

    for(int i=0; i<120; i++)
    {
        yf_position[i] = yf_position_line[i].toFloat();
        //        qDebug()<<"yf_position = "<<i<<"  = "<<yf_position_line[i].toFloat();
    }










    //加载pileUp参数
    QFile pileUp_file("pileUp.txt");
    QString pileUp_line[10];
    if (pileUp_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&pileUp_file);
        int i = 0;
        while (!in.atEnd())
        {
            pileUp_line[i] = in.readLine();
            i++;
        }
        pileUp_file.close();

        if(i<1)
        {
            log_str = "[load conf file error]:pileUp_filetxt";
            emit Display_log_signal(log_str);
        }else {

            QString lineText_1 = pileUp_line[0];
            QString lineText_2 = pileUp_line[1];
            QStringList lineTextList_1 = lineText_1.split(",");
            QStringList lineTextList_2 = lineText_2.split(",");
            int len1 = lineTextList_1.length();
            int len2 = lineTextList_2.length();
            if(len1 != len2)
            {
                log_str = "[load conf file error]:pileUp_filetxt";
                emit Display_log_signal(log_str);
                return;

            }
            cal.clear();
            val.clear();
            for(int i=0; i<len1;i++)
            {
                cal.push_back(lineTextList_1[i].toFloat());
                val.push_back(lineTextList_2[i].toFloat());
            }


            qDebug()<<"length = "<<cal.size();
            for(int i=0;i<cal.size();i++)
            {
                qDebug()<<" i="<<i<<"  "<<cal[i]<<"  "<<val[i];
            }



            log_str = "[load conf file success]:pileUp_file.txt";
            emit Display_log_signal(log_str);
        }

    }else{
        log_str = "[load conf file error]:pileUp_file.txt";
        emit Display_log_signal(log_str);
    }




    //读取相机的内参与畸变参数
    QSettings paraSetting("setting.ini", QSettings::IniFormat);
    fx = paraSetting.value("camera_calibration/fx").toString().toFloat();
    fy = paraSetting.value("camera_calibration/fy").toString().toFloat();
    cx = paraSetting.value("camera_calibration/cx").toString().toFloat();
    cy = paraSetting.value("camera_calibration/cy").toString().toFloat();
    k1 = paraSetting.value("camera_calibration/k1").toString().toFloat();
    k2 = paraSetting.value("camera_calibration/k2").toString().toFloat();

    qDebug()<<" fx="<<fx<<" fy="<<fy<<" cx="<<cx<<" cy="<<cy<<" k1="<<k1<<"  k2="<<k2;
    if(fx<1 && fy<1 && cx<1 && cy<1 && k1<1 && k2<1)
    {
        fx = 169.7123;
        fy = 162.7944;
        cx = 80.5252;
        cy = 59.9091;
        k1 = 0.3411;
        k2 = -1.4510;

        log_str = "[load conf file error]:Setting.ini";
        emit Display_log_signal(log_str);

    }else
    {
//        log_str = "[load conf file success]:Setting.ini";
//        emit Display_log_signal(log_str);
    }


}

//!
//! \brief DealUsb_msg::change_gain_slot
//! \param gain
//!改变图像的增益
void DealUsb_msg::change_gain_slot(float gain)
{
    gainImage = gain;
}

//!
//! \brief change_tof_peak_slot
//!切换tof/peak的槽函数
void DealUsb_msg::change_tof_peak_slot()
{
    if(isTOF == true)
        isTOF = false;
    else
        isTOF = true;
}

//修改统计帧数的槽函数
void DealUsb_msg::alterStatisticFrameNum_slot(int num)
{
    statisticFrameNumber = num ;

}

//修改是否进行滤波的槽函数
void DealUsb_msg::isFilter_slot(bool isFiter)
{
    if(true == isFiter)
    {
        isFilterFlag = true;
    }else{
        isFilterFlag = false;
    }
}


void DealUsb_msg::alter_focal_integrate_slot(float focal_length,float tx_rx_diff,int integrateNum)   //修改焦距、积分次数的槽函数
{
    f = focal_length;    //焦距
    camera_dis = tx_rx_diff;   //tx_RX间距
    integrateNum_ = integrateNum;    // 公式为  peak/积分次数 * 100

    qDebug()<<"f = "<<f;
    qDebug()<<"camers_diff = "<<camera_dis<<"   jifenNum="<<integrateNum_;

}











////! \brief DealUsb_msg::tof_filter
////!中值滤波算法的实现
//void DealUsb_msg::tof_filter()
//{
//    vector<int> sortArray;
//    vector<int> sortArray_3D;

//    QRgb tofColor;
//    int gainIndex_tof;
//    int cloudIndex;

//    int winSize = 2;
//    int win3D_size = 4;

//    for(int imgX=0; imgX<160; imgX++)
//    {
//        for(int imgY=0; imgY<120; imgY++)
//        {
//            for(int i=-winSize/2;i<winSize/2;i++)
//            {
//                for(int j=-winSize/2;j<winSize/2;j++)
//                {
//                    if(imgX+i>=0 && imgX+i<160 && imgY+j>=0 && imgY+j<120)
//                    {
//                        sortArray.push_back(src_tof[imgX+i][imgY+j]);   //存储符合peak的值
//                    }
//                }
//            }
//            int len = sortArray.size();

//            if(len>0)
//            {
//                sort(sortArray.begin(),sortArray.end());
//                int midNumTof = sortArray[len/2];   //获取到的中值

//                gainIndex_tof = midNumTof * gainImage_tof;
//                if(gainIndex_tof<1024 && gainIndex_tof>=0)
//                    tofColor = qRgb(colormap[gainIndex_tof * 3], colormap[gainIndex_tof * 3 + 1], colormap[gainIndex_tof * 3 + 2]);
//                else
//                    tofColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);
//                microQimage.setPixel(imgX,imgY,tofColor);         //TOF图像的赋值

//            }
//            sortArray.clear();

//            //以上是对二维图像采用的 窗口大小为3的窗口的滤波

//            for(int m=-win3D_size/2; m<win3D_size/2; m++ )
//            {
//                for(int n=-win3D_size/2; n<win3D_size/2; n++)
//                {
//                    if(imgX+m>=0 && imgX+m<160 && imgY+n>=0 && imgY+n<120)
//                    {
//                        sortArray_3D.push_back(src_tof[imgX+m][imgY+n]);

//                    }
//                }
//            }
//            int len_3D = sortArray_3D.size();
//            if(len_3D > 0)
//            {
//                sort(sortArray_3D.begin(),sortArray_3D.end());
//                int midNumTof_3D = sortArray_3D[len_3D/2];   //获取到的中值

//                gainIndex_tof = midNumTof_3D * gainImage_tof;
//                if(gainIndex_tof<1024 && gainIndex_tof>=0)
//                    tofColor = qRgb(colormap[gainIndex_tof * 3], colormap[gainIndex_tof * 3 + 1], colormap[gainIndex_tof * 3 + 2]);
//                else
//                    tofColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);


//                //更新点云数据的值 以及颜色
//                cloudIndex = imgY*160 + imgX;
//                temp_y = calibration_y(midNumTof_3D,imgX,imgY);         //tof转换depth公式
//                temp_x = calibration_x(temp_y,imgX,imgY);
//                temp_z = calibration_z(temp_y,imgX,imgY);

//                //点云数据做校正integrateNum_
//                temp_y = temp_y + tofOffsetArray[cloudIndex];
//                temp_x = temp_x + calibration_x(tofOffsetArray[cloudIndex],imgX,imgY);
//                temp_z = temp_z + calibration_z(tofOffsetArray[cloudIndex],imgX,imgY);

//                if(temp_y<0)
//                {
//                    temp_y = 0;
//                    temp_x = 0;
//                    temp_z = 0;
//                }

//                QColor mColor = QColor(tofColor);
//                r = mColor.red();
//                g = mColor.green();
//                b = mColor.blue();
//                rgb = ((int)r << 16 | (int)g << 8 | (int)b);
//                tempRgbCloud.points[cloudIndex].x = temp_x;
//                tempRgbCloud.points[cloudIndex].y = temp_y;
//                tempRgbCloud.points[cloudIndex].z = temp_z;
//                tempRgbCloud.points[cloudIndex].rgb = *reinterpret_cast<float*>(&rgb);
//            }
//            sortArray_3D.clear();


//        }//imgY
//    }//imgX

//}

//! \brief DealUsb_msg::tof_filter
//!中值滤波算法的实现
void DealUsb_msg::tof_filter()
{
    vector<int> sortArray;
    vector<int> sortArray_3D;


    int winSize = 2;
    int win3D_size = 4;

    for(int imgX=0; imgX<160; imgX++)
    {
        for(int imgY=0; imgY<120; imgY++)
        {
            for(int i=-winSize/2;i<winSize/2;i++)
            {
                for(int j=-winSize/2;j<winSize/2;j++)
                {
                    if(imgX+i>=0 && imgX+i<160 && imgY+j>=0 && imgY+j<120)
                    {
                        sortArray.push_back(src_tof[imgX+i][imgY+j]);   //存储符合peak的值
                    }
                }
            }
            int len = sortArray.size();

            if(len>0)
            {
                sort(sortArray.begin(),sortArray.end());
                int midNumTof = sortArray[len/2];   //获取到的中值
                afterMid_tof[imgX][imgY] = midNumTof;     //中值滤波后的结果
            }
            sortArray.clear();

         //以上是对二维图像采用的 窗口大小为3的窗口的滤波
        }//imgY
    }//imgX

}


//! 畸变校正的槽函数
//!  输出结果保存到矩阵     float undistorted_tof[160][120];
//!                        float undistorted_peak[160][120];
void DealUsb_msg::distortionCorrect_slot()
{
//    double k1 = 0.3411;
//    double k2 = -1.4510;
//    double p1 = 0;
//    double p2 = 0;
//    double fx = 169.7123;
//    double fy = 162.7944;
//    double cx = 80.5252;
//    double cy = 59.9091;



    double u_distorted = 0;
    double v_distorted = 0;

    double XC1,YC1,XC2,YC2,r2;

    for(int v=0; v<120; v++)
    {
        for(int u=0;u<160;u++)
        {
            u_distorted = 0;
            v_distorted = 0;

            XC1 = (u-cx)/fx;
            YC1 = (v-cy)/fy;

            r2 = pow(XC1,2) + pow(YC1,2);
            XC2 = XC1*(1+ k1*r2 + k2*pow(r2,2) ) ;
            YC2 = YC1*(1+ k1*r2 + k2*pow(r2,2) ) ;

            u_distorted = fx * XC2 + cx;
            v_distorted = fy * YC2 + cy;

            if(u_distorted >= 0 && v_distorted >= 0 && u_distorted < 160 && v_distorted < 120)
            {
                int current_row =  (v_distorted);
                int current_col = (u_distorted);

                undistorted_tof[u][v] = afterMid_tof[current_col][current_row];
                undistorted_peak[u][v] = src_peak[current_col][current_row];

            }else
            {
                undistorted_tof[u][v] = 0;
                undistorted_peak[u][v] = 0;
            }

        }
    }
}



//!
//! \brief DealUsb_msg::UV910_Qtech_deal_slot
//!  接收UV910 QTech数据  Q_tech 模组每帧数据的大小为 48000个字节  对应于 48000*（4/5） = 38400个数据 对应于19200个TOF 和PEAK
void DealUsb_msg::UV910_Qtech_deal_slot(QByteArray array)
{
    int rowImg,colImg;
    float tof,row_tof,after_pileup_tof,after_kalman_tof,intensity,src_intensity,raw_intensity,after_kalman_intensity,peak;
    int byteIndex = 0;       //一个byteIndex包含4个数据， 2个tof、2个peak
    int lineIndex = 0;       //0-29  宏像素所在的行,0,1,2,...29   宏像素的第几个 因为是4个为一组   四个像素点构成一个宏像素
    int InnerLineNum = 0;    //属于宏像素的第一行还是第二行； spadNum%4 = 0,1,2,3  0、1在第一行  2、3在第二行
    int tmpRow = 0;

//    QTime t1,t2;
//    t1 = QTime::currentTime();
    for(int i=0; i<120; i++)
    {
        lineIndex = i/4;
        InnerLineNum = (i%4)/2;
        tmpRow = lineIndex*2 + InnerLineNum;
        for(int j=0; j<80; j++)
        {
            for(int k=0; k<2; k++)
            {
                if(0 ==k)
                {
                    tof = ((quint8(array[byteIndex*5+0])<<2) + ((quint8(array[byteIndex*5+4])&0xC0)>>6));
                    row_tof = tof;
                    intensity = ((quint8(array[byteIndex*5+1])<<2) + ((quint8(array[byteIndex*5+4])&0x30)>>4));
                    src_intensity = intensity;
                    intensity = intensity/integrateNum_*100;
                    raw_intensity = intensity;
                }else
                {
                    tof = ((quint8(array[byteIndex*5+2])<<2) + ((quint8(array[byteIndex*5+4])&0x0C)>>2));
                    row_tof = tof;
                    intensity = ((quint8(array[byteIndex*5+3])<<2) + ((quint8(array[byteIndex*5+4])&0x03)));
                    src_intensity = intensity;
                    intensity = intensity/integrateNum_*100;
                    raw_intensity = intensity;
                }


                if(j<40)    //上半区的像素
                {
                    rowImg = 59-tmpRow;
                    colImg = j*4 + k*2 + i%2;

                }else      //下半区的像素
                {
                    rowImg = 60+tmpRow;
                    colImg = (j-40)*4 + k*2 + i%2;
                }

                cloudIndex = rowImg*160 + colImg;



                //1、首先对peak做kalman滤波
//                if(isKalman)
//                {
//                    intensity = kalmanFilter_peak_slot(raw_intensity,cloudIndex);
//                }
                after_kalman_intensity = intensity;


                // 2、利用Peak值线性外插法可得对应校正量ΔTOF (Unit: mm)
                if(is_pileUp_flag)
                {
                    tof = pileUp_calibration(tof,intensity);
                }
                after_pileup_tof = tof;


                //3、对tof进行kalman滤波
                if(isKalman)
                {
                    tof = kalmanFilter_tof_slot(tof,cloudIndex,intensity);
                }
                after_kalman_tof = tof;


                //4、 统计均值 、方差相关**********  此处的统计都是基于 最原始的TOF 和 PEAK 值做的处理*************/
                if(statisticStartFlag == true)
                {
                    //判断每个点已经储存的个数，如果已经超过设定的范围，则进行循环储存；
                    int offset = tempStatisticPeakPoints[cloudIndex].size() - statisticFrameNumber;
                    if(offset >= 0)
                    {
                        tempStatisticTofPoints[cloudIndex].erase(tempStatisticTofPoints[cloudIndex].begin(),tempStatisticTofPoints[cloudIndex].begin() + offset + 1);
                        tempStatisticPeakPoints[cloudIndex].erase(tempStatisticPeakPoints[cloudIndex].begin(),tempStatisticPeakPoints[cloudIndex].begin() + offset + 1);
                    }
                    //向每个点的容器中添加一个新的点,完成循环存储
                    tempStatisticTofPoints[cloudIndex].push_back(row_tof);
                    tempStatisticPeakPoints[cloudIndex].push_back(src_intensity);
                }

                src_tof[colImg][rowImg] = tof;
                afterMid_tof[colImg][rowImg] = tof;
                src_peak[colImg][rowImg] = intensity;


//                //4、 循环赋值取平均值  部分，默认不开启
//                if(meanFilter_flag)
//                {
//                    for(int n=0;n<averageNum-1;n++)
//                    {
//                        lastTOF[n][cloudIndex] = lastTOF[n+1][cloudIndex];
//                    }
//                    lastTOF[averageNum-1][cloudIndex] = tof;
//                    if(haveIndex >averageNum)
//                    {
//                        if(abs(tof-mean_tof)>meanFilter_offset)
//                        {
//                            temp_x = 0;
//                            temp_y = 0;
//                            temp_z = 0;
//                        }

//                        float zeroNum = 0;
//                        haveIndex = averageNum+1;
//                        float allTof_100 = 0;
//                        for(int k=0; k<averageNum; k++)     //100帧取平均   ，如果有0的数据则不进行平均处理
//                        {
//                            if(lastTOF[k][cloudIndex] == 0)
//                            {
//                                zeroNum = zeroNum+1;
//                            }
//                            allTof_100 += lastTOF[k][cloudIndex];
//                        }
//                        if(zeroNum != averageNum)

//                            mean_tof = allTof_100/(averageNum-zeroNum);
//                    }
//                }
                //////////////////////////////////////////////////////



                // 13、  文件保存相关*****************/
                if(isSaveFlag == true)
                {
                    if(isSaveRawTof)
                    {
                        tofPeakNum[cloudIndex+1] = QString::number(row_tof) + " " ;
                    }
                    if(isSaveRawPeak)
                    {
                        tofPeakNum[cloudIndex+1].append(QString::number(raw_intensity) + " " );
                    }

                }

            }  // for K
            byteIndex++;  //开启下一个包  （5个字节包含4个数据）

        } //for j

    } //for i    一帧数据已经接收完毕



    haveIndex++;
    if(haveIndex>3)
        haveIndex = averageNum+1;



    // 5 统计信息相关的 ，将统计信息的容器赋值给全局变量
    if(statisticStartFlag)
    {
        statisticMutex.lock();
        allStatisticTofPoints = tempStatisticTofPoints;
        allStatisticPeakPoints = tempStatisticPeakPoints;
        statisticMutex.unlock();
    }

    // 6 中值滤波算法
    if(true == isFilterFlag)
    {
        tof_filter();
    }

    // 7 畸变校正
    if(1)
    {
        distortionCorrect_slot();
    }


    //遍历undistorted_tof矩阵，做如下处理： 坐标转换、offset校正、、文件保存、二维/三维图像赋值
    for(int rowImg=0; rowImg<120; rowImg++)
    {
        for(int colImg=0; colImg<160; colImg++)
        {
            cloudIndex = rowImg*160 + colImg;

            tof = undistorted_tof[colImg][rowImg];
            peak = undistorted_peak[colImg][rowImg];
//            tof = src_tof[colImg][rowImg];
//            peak = src_peak[colImg][rowImg];

//            tof = afterMid_tof[colImg][rowImg];

            // 8 坐标转换
            temp_y = calibration_y(tof,colImg,rowImg);
            temp_x = calibration_x(temp_y,colImg,rowImg);
            temp_z = calibration_z(temp_y,colImg,rowImg);

            // 9  添加offset
            temp_y = temp_y + tofOffsetArray[cloudIndex];
            temp_x = temp_x + calibration_x(tofOffsetArray[cloudIndex],colImg,rowImg);
            temp_z = temp_z + calibration_z(tofOffsetArray[cloudIndex],colImg,rowImg);
            if(temp_y<0 || peak<peakOffset)
            {
                temp_x=0;
                temp_y=0;
                temp_z=0;
            }

            // 10 自动校准相关
            if(true == isAutoCalibration_flag)
            {
                float mean_res_y = 0;
                vec_mean_y[cloudIndex].push_back(temp_y);
                if(vec_mean_y[cloudIndex].size() == calibration_mean_num)
                {
                    mean_res_y = std::accumulate(std::begin(vec_mean_y[cloudIndex]),std::end(vec_mean_y[cloudIndex]),0.0)/calibration_mean_num;
                    calibrate_offset_slot(cloudIndex,mean_res_y);
                    vec_mean_y[cloudIndex].clear();
                }
            }

            // 11、 文件保存相关
            if(isSaveFlag)
            {
                if(isSavePileUpTof)
                {
                    tofPeakNum[cloudIndex+1].append(QString::number(after_pileup_tof) + " " );
                }
                if(isSaveFilterTof)
                {
                    tofPeakNum[cloudIndex+1].append(QString::number(tof) + " " );     //卡尔曼滤波以后 对像素进行了平移处理
                }
                if(isSaveX)
                {
                    tofPeakNum[cloudIndex+1].append(QString::number(temp_x) + " ");
                }
                if(isSaveY)
                {
                    tofPeakNum[cloudIndex+1].append(QString::number(temp_y) + " ");
                }
                if(isSaveZ)
                {
                    tofPeakNum[cloudIndex+1].append(QString::number(temp_z) + " ");
                }
            }


            //12、 鼠标显示相关
            mouseShowMutex.lock();
            mouseShowTOF[colImg][rowImg] = tof;
            mouseShowPEAK[colImg][rowImg] = peak;
            mouseShowDepth[colImg][rowImg] = temp_y;
            mouseShow_X[colImg][rowImg] = temp_x;
            mouseShow_Z[colImg][rowImg] = temp_z;
            mouseShowMutex.unlock();



//            qDebug()<<" cloudIndex =  "<<cloudIndex;

            // 13、二维、三维图像赋值
            QRgb tofColor,intenColor;
            int gainIndex_tof = tof*gainImage_tof;
            int gainIndex_intensity =peak * gainImage_peak;
            if(gainIndex_tof<1024 && gainIndex_tof>=0)
                tofColor = qRgb(colormap[gainIndex_tof * 3], colormap[gainIndex_tof * 3 + 1], colormap[gainIndex_tof * 3 + 2]);
            else
                tofColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);

            if(gainIndex_intensity<1024 && gainIndex_intensity>=0)
                intenColor = qRgb(colormap[gainIndex_intensity * 3], colormap[gainIndex_intensity * 3 + 1], colormap[gainIndex_intensity * 3 + 2]);
            else
                intenColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);

            microQimage.setPixel(colImg,rowImg,tofColor);         //TOF图像的赋值
            macroQimage.setPixel(colImg,rowImg,intenColor);       //强度图像的赋值

            QColor mColor = QColor(tofColor);
            r = mColor.red();
            g = mColor.green();
            b = mColor.blue();
            rgb = ((int)r << 16 | (int)g << 8 | (int)b);
            tempRgbCloud.points[cloudIndex].x = temp_x;
            tempRgbCloud.points[cloudIndex].y = temp_y;
            tempRgbCloud.points[cloudIndex].z = temp_z;
            tempRgbCloud.points[cloudIndex].rgb = *reinterpret_cast<float*>(&rgb);

        }
    }


    //13、赋值给全局变量
    mutex_3D.lock();
    tofImage = microQimage;
    intensityImage = macroQimage;
    pcl::copyPointCloud(tempRgbCloud,pointCloudRgb);
    mutex_3D.unlock();



    // 14 判断是否保存数据
    if(isSaveFlag)
    {
        bool flag = true;
        for(int i=0; i<19201; i++)
        {
            if(i>0)
            {
                tofPeakNum[i].append("\n");
            }
            tofPeakToSave_string.append(tofPeakNum[i]);
            //tofPeakNum[i].clear();    //这句代码屏蔽以后 ，当有遗漏数据的时候，会使用上一帧的数据进行存储
        }
        emit saveTXTSignal(tofPeakToSave_string);
        tofPeakToSave_string.clear();
    }

    isShowPointCloud = true;

}







//!   为提高计算的精度 ，计算过程中统一采用毫米单位  最后转换为m来进行显示
//! \brief DealUsb_msg::calibration_y
//! \param cal_tof  单位LSB
//! \param x_pix    单位微米
//! \param y_pix    单位微米
//! \return
//!//tof值 x_pix的位置标号  y_pix的位置标号
float DealUsb_msg::calibration_y(float cal_tof,int x_pix,int y_pix)
{
//    if((cal_tof* LSB *1000) <= camera_dis)
//        return 0;

//    float tofLSB = cal_tof/2.0 * LSB *1000;   //mm       ps:校正公式中的tof为接收到的真实TOF的一半    03-20 alter
//    float tmp1 = 4*tofLSB*tofLSB - camera_dis*camera_dis;
//    float tmp2 = 4*tofLSB*sqrt(pow(xf_position[x_pix]*miu_meter/f,2) + pow(yf_position[y_pix]*miu_meter/f,2) + 1 ) + 2*xf_position[x_pix]*miu_meter*camera_dis/f;
//    float y = tmp1/tmp2;
//    y = y/1000.0;   //mm->m
//    return y;

//    double k1 = 0.3411;
//    double k2 = -1.4510;
//    double p1 = 0;
//    double p2 = 0;
//    double fx = 169.7123;
//    double fy = 162.7944;
//    double cx = 80.5252;
//    double cy = 59.9091;
    float tofLSB = cal_tof/2.0 * LSB *1000;//mm       ps:校正公式中的tof为接收到的真实TOF的一半    03-20 alter
    float tmp1 = pow(tofLSB,2);
    float tmp2 = pow(x_pix-cx,2)/pow(fx,2) + pow(y_pix-cy,2)/pow(fy,2) + 1;

    float y = sqrt(tmp1/tmp2)/1000.0;
    return y;

}

//!//!   为提高计算的精度 ，计算过程中统一采用毫米单位  最后转换为m来进行显示
//! \brief DealUsb_msg::calibration_x
//! \param cal_y
//! \param x_pix
//! \param y_pix
//! \return
//! //计算后的y的值  x_pix的位置标号  y_pix的位置标号
float DealUsb_msg::calibration_x(float cal_y,int x_pix,int y_pix)
{
//    cal_y = cal_y*1000;   //m->mm
//    float x = -1 * cal_y * xf_position[x_pix]*miu_meter/f;
//    x = x/1000.0;      //mm->m
//    return -x;


    cal_y = cal_y * 1000;
    float x = (x_pix-cx)*cal_y/fx;
    return x/1000.0;

}

//! //!   为提高计算的精度 ，计算过程中统一采用毫米单位  最后转换为m来进行显示
//! \brief DealUsb_msg::calibration_z
//! \param cal_y
//! \param x_pix
//! \param y_pix
//! \return
//!  //计算后的z的值  x_pix的位置标号  y_pix的位置标号
float DealUsb_msg::calibration_z(float cal_y,int x_pix,int y_pix)
{
//    cal_y = cal_y*1000;
//    float z = -1 * cal_y *yf_position[y_pix]*miu_meter/f;
//    z = z/1000.0;      //mm->m
//    return z;

    cal_y = cal_y * 1000;
    float z = (y_pix-cy)*cal_y/fy;
    return -z/1000.0;

}

//!
//! \brief DealUsb_msg::pileUp_calibration
//! \param srcTof
//! \param peak
//! \return
//!利用Peak值线性外插法可得对应校正量ΔTOF (Unit: mm)
float DealUsb_msg::pileUp_calibration(int srcTof,float peak)
{
    if(cal.size()<1 || val.size()<1)
        return srcTof;

    float a = cal[0];
    float b = val[0];
    float bias_tof_e = a * exp(b*peak)/31.0;
    float resTof_e = srcTof + bias_tof_e;
    return resTof_e;



    //4310 pileUp参数更新
//    float cal[] = {0, 18.608, 27.9257, 42.1217, 54.9701, 67.1999, 81.18035, 91.876348, 96.589933, 98.880245, 99.529313, 99.7605085, 99.9482045, 110};
//    float val[] = {0,  0,     0,    0,   36.45,   49.755,  124.285,   160.995,   285.605,   427.575,  448.335,   503.65,      673.385,   700};
//    float begin_tof,end_tof,offset_start,offset_end;
//    float resTof = 0;
//    float bias_tof = 0;
//    int len = sizeof(cal)/sizeof(cal[0]);

//    float cal[] = {0, 18.608, 27.9257, 42.1217, 54.9701, 67.1999, 81.18035, 91.876348, 96.589933, 98.880245, 99.529313, 99.7605085, 99.9482045, 110};
//    float val[] = {0,  0,     0,    0,   36.45,   49.755,  124.285,   160.995,   285.605,   427.575,  448.335,   503.65,      673.385,   700};
    float begin_tof,end_tof,offset_start,offset_end;
    float resTof = 0;
    float bias_tof = 0;
    int len = cal.size();



//    qDebug()<<"len = "<<len<<" 0"<<cal[len-2];
//    vector<float> cal;
//    vector<float> val;

    for(int i=0; i<len-1; i++)
    {
        if(peak>=cal[i] && peak<cal[i+1])
        {
            begin_tof = cal[i];
            end_tof = cal[1+i];
            offset_start = val[i]/31.0;
            offset_end = val[1+i]/31.0;
            bias_tof = (peak-begin_tof)*(offset_end-offset_start)/(end_tof-begin_tof) + offset_start;
            resTof = srcTof + bias_tof;
            //            qDebug()<<"resTof = "<<resTof;
            return  resTof;
        }
    }
    return srcTof + val[len-1]/31.0;

}

//!
//! \brief DealUsb_msg::kalmanFilter_peak_slot
//! \param srcPeak   原始peak值
//! \param index    点云序号
//! \return  滤波后的peak值
//!
float DealUsb_msg::kalmanFilter_peak_slot(float srcPeak,int index)
{

    float p1 = -0.01034;
    float p2 = 1.032;
    float p3 = -0.04257;

    if(0 == haveIndex)
    {
        peak_x_k[index] = srcPeak;
        return peak_x_k[index];
    }

    peak_kalman_R = p1 *srcPeak*srcPeak + p2*srcPeak+ p3;             //取值范围[1 15]
    peak_kalman_R = peak_kalman_R<1?1:peak_kalman_R;

    //更新 K[index]
    peak_p_k[index] = peak_kalman_F * peak_p_k[index] *  peak_kalman_F + peak_kalman_Q;
    peak_K[index] = peak_p_k[index] * peak_kalman_H /(peak_kalman_H * peak_p_k[index] * peak_kalman_H + peak_kalman_R);
    peak_p_k[index] = (1 - peak_K[index] * peak_kalman_H) * peak_p_k[index];
    //更新 peak_x_k[index]
    peak_x_k[index] = peak_kalman_F * peak_x_k[index];
    peak_x_k[index] = peak_x_k[index] + peak_K[index]*(srcPeak - peak_kalman_H * peak_x_k[index]);


    //    qDebug()<<"srcPeak="<<srcPeak<<"  resPeak="<<peak_x_k[index];
    return peak_x_k[index] ;
}


//!
//! \brief DealUsb_msg::kalmanFilter_slot
//! \param srcTof   原始Tof值
//! \param index    标号（0-19119）
//! \return peak    强度值
//! \return         滤波算法以后的tof
//! 卡尔曼滤波算法  TOF
float DealUsb_msg::kalmanFilter_tof_slot(float srcTof,int index,float peak)
{

    //    qDebug()<<"kalmanFilter_tof_slot(float srcTof,int index,float peak)";

    if(0 == haveIndex)   //第一帧数据,初始化xk为观测值
    {
        x_k[index] = srcTof;
        return x_k[index];
    }

    float tof_offset = kalmanOffset_para*(10.86/sqrt(peak)+0.22) ;

    float tmpTof = x_k[index];

    //    if(abs(srcTof-x_k[index])>tof_offset)
    //        return srcTof;



    kalman_R = 85.0/peak;               //取值范围[1 15]170
    kalman_R = kalman_R<1?1:kalman_R;
    kalman_R = kalman_R>15?15:kalman_R;

    //    kalman_R = 5;

    //更新 K[index]
    p_k[index] = kalman_F * p_k[index] *  kalman_F + kalman_Q;
    K[index] = p_k[index] * kalman_H /(kalman_H * p_k[index] * kalman_H + kalman_R);
    p_k[index] = (1 - K[index] * kalman_H) * p_k[index];
    //更新 x_k[index]
    x_k[index] = kalman_F * x_k[index];
    x_k[index] = x_k[index] + K[index]*(srcTof - kalman_H * x_k[index]);

    //    return x_k[index];

    if(abs(srcTof-tmpTof)>tof_offset)
    {
        return srcTof;
    }else
    {
        return x_k[index];
    }


}

//!
//! \brief autoCalibration_Dialog::on_startCalibration_pushButton_clicked
//! 1、开始校正
//! 2、发送信号到数据处理线程
//! 3、首先清空tofOffsetArray数组，设置为0
//! 4、设置校正的 flag = true
//! 5、把 59*160+79=9519        59*160+80=9520   60*160+79= 9679  60*160+80=9680 的y值传入给处理函数                         （31 32）/（127 128）
//! 6、数据处理函数首先判断够不够100帧，满100帧时，分别取y的均值，分别逆运算为resTof
//! 7、计算理论的realTof值，根据用户指定的距离(单位:m);
//! 8、offset = realTof - resTof
//! 9、写入本地文件，程序重新设置tofOffsetArray数组，isAutoCalibration_flagfalse
void  DealUsb_msg::start_autoCalibration_slot(float meters)
{
    for(int i=0;i<19200;i++)
    {
        tofOffsetArray[i] = 0;
    }
    calibration_real_dis = meters;
    isAutoCalibration_flag = true;
    qDebug()<<"  calibration_real_dis = "<<calibration_real_dis;

}

//!
//! \brief DealUsb_msg::calibrate_offset_slot
//!  计算校验的offset
//! 1、获取100帧数据的y坐标的均值（中间4个点的）
//! 2、根据公式反推出realTof
//! 3、offset = realTof - tof
//!
//!
//!
//!
//! 59*160+79=9519        59*160+80=9520   60*160+79= 9679  60*160+80=9680


//  x = realTof*LSB*1000
//! y = calibration_real_dis *1000
//! a = camera_dis
//! M = xf*miu_meter/f
//! N = yf*miu_meter/f
//!
//! tmp1 = sqrt((pow(a,2) + 2*a*M*y + pow(y,2)*(M*M+N*N+1))/4)
//! tmp2 = y*sqrt(M*M+N*N+1)/2.0
//!
//! x = tmp2 + tmp1   ||  x= tmp2-tmp1
//!
//! 经过测试 tmp2<tmp1  所以最终结论是 x= tmp2+tmp1
void DealUsb_msg::calibrate_offset_slot(int index,float mean_tof)
{
    ishave_Four++;

    float res_y_offset ;
    if(mean_tof<1)
    {
        res_y_offset = 0;
    }else
    {
         res_y_offset = calibration_real_dis - mean_tof;
    }



    y_offset[index] = QString::number(res_y_offset);
    if(ishave_Four == 19200)
    {
        QFile file("tofOffsetArray.txt");
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream out(&file);
        for(int i=0; i<19200; i++)
        {
            tofOffsetArray[i] = y_offset[i].toFloat();
            out<<y_offset[i].toLocal8Bit()<<endl;
        }

        isAutoCalibration_flag = false;
        ishave_Four = 0;
        QString msg = "raw_tof="+QString::number(calibration_real_dis)+",dst_tof="+QString::number(mean_tof)+",offset="+QString::number(res_y_offset);

        QString DisplayNote = "[Auto Calibration success]:"+msg;
        emit send_cali_success_signal(msg);         //发送给设置自动校准的界面
        emit Display_log_signal(DisplayNote);       //在日志显示窗口显示校准信息
    }
}



/********************************* **************** ****************  read local_file ************************ **************** **************** **************** *****************************/

void DealUsb_msg::playLocalFile_slot(QString sPath)
{
    filePath = sPath;

    if( NULL == localFile_timer)
    {
        localFile_timer = new QTimer();
        connect(localFile_timer,SIGNAL(timeout()),this,SLOT(readLocalPCDFile()));
        localFile_timer->start(80);
    }
    fileIndex = 1;


}

void DealUsb_msg::readLocalPCDFile()
{

    QString fileName;
    QString line[50000];
    QStringList tofPeakList;
    int imgRow,imgCol;

    fileName = filePath + QString::number(fileIndex)+".txt";
    fileIndex++;

    QFile file(fileName);

    int countNum = 0;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while (!in.atEnd())
        {
            line[countNum] = in.readLine();
            countNum++;
        }
        file.close();
    }else
    {
        qDebug()<<"read file error!"<<endl;
        //            localFile_timer->stop();
        fileIndex = 1;
        return;
    }


    for(int i=1; i<countNum-1; i++)            //去掉空的数据
    {
        int tof,intensity;
        if(line[i].isEmpty())
            continue;

        tofPeakList = line[i].split(" ");

        if(isTOF)
        {
            tof = tofPeakList[0].toFloat();
            intensity = tofPeakList[1].toFloat();
        }else
        {
            intensity = tofPeakList[0].toFloat();
            tof = tofPeakList[1].toFloat();
        }

        if(is_pileUp_flag)
        {
            tof = pileUp_calibration(tof,intensity);
        }


        //行列以及颜色传递给图像
        imgRow = (i-1)%160;
        imgCol = (i-1)/160;
        cloudIndex = i-1;      //在点云数据中的标号

//            qDebug()<<"imgRow="<<imgRow<<"  imgCol="<<imgCol<<endl;

        //设置TOF图像、强度图像的颜色
        QRgb tofColor,intenColor;
        int gainIndex_tof = tof*gainImage_tof;
        int gainIndex_intensity =intensity * gainImage_peak;

        //        int gainIndex_tof = tof *0.5;
        //        int gainIndex_intensity = intensity * 0.5;

        if(gainIndex_tof<1024 && gainIndex_tof>=0)
            tofColor = qRgb(colormap[gainIndex_tof * 3], colormap[gainIndex_tof * 3 + 1], colormap[gainIndex_tof * 3 + 2]);
        else
            tofColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);

        if(gainIndex_intensity<1024 && gainIndex_intensity>=0)
            intenColor = qRgb(colormap[gainIndex_intensity * 3], colormap[gainIndex_intensity * 3 + 1], colormap[gainIndex_intensity * 3 + 2]);
        else
            intenColor = qRgb(colormap[1023 * 3], colormap[1023 * 3 + 1], colormap[1023 * 3 + 2]);


        if(imgRow>=0 && imgRow<160 && imgCol>=0 && imgCol<120)
        {
            microQimage.setPixel(imgRow,imgCol,tofColor);         //TOF图像的赋值
            macroQimage.setPixel(imgRow,imgCol,intenColor);       //强度图像的赋值




            /************点云数据相关************/
            //获取三维坐标
            temp_y = calibration_y(tof,imgRow,imgCol);
            temp_x = calibration_x(temp_y,imgRow,imgCol);
            temp_z = calibration_z(temp_y,imgRow,imgCol);

            QColor mColor = QColor(tofColor);
            r = mColor.red();
            g = mColor.green();
            b = mColor.blue();
            rgb = ((int)r << 16 | (int)g << 8 | (int)b);


            tempRgbCloud.points[cloudIndex].x = temp_x;
            tempRgbCloud.points[cloudIndex].y = temp_y;
            tempRgbCloud.points[cloudIndex].z = temp_z;
            tempRgbCloud.points[cloudIndex].rgb = *reinterpret_cast<float*>(&rgb);

            //            qDebug()<<" cloudIndex = "<<cloudIndex<<endl;
            /************鼠标点击处显示信息相关*************/
            mouseShowMutex.lock();
            mouseShowTOF[imgRow][imgCol] = temp_y;
            mouseShowPEAK[imgRow][imgCol] = intensity;
            mouseShowDepth[imgRow][imgCol] = temp_y;
            mouseShow_X[imgRow][imgCol] = temp_x;
            mouseShow_Z[imgRow][imgCol] = temp_z;
            mouseShowMutex.unlock();




            /***************统计均值 、方差相关***********************/
            if(statisticStartFlag == true)
            {
                //判断每个点已经储存的个数，如果已经超过设定的范围，则进行循环储存；
                int offset = tempStatisticPeakPoints[cloudIndex].size() - statisticFrameNumber;
                if(offset >= 0)
                {
                    tempStatisticTofPoints[cloudIndex].erase(tempStatisticTofPoints[cloudIndex].begin(),tempStatisticTofPoints[cloudIndex].begin() + offset + 1);
                    tempStatisticPeakPoints[cloudIndex].erase(tempStatisticPeakPoints[cloudIndex].begin(),tempStatisticPeakPoints[cloudIndex].begin() + offset + 1);
                }
                //向每个点的容器中添加一个新的点,完成循环存储
                tempStatisticTofPoints[cloudIndex].push_back(tof);
                tempStatisticPeakPoints[cloudIndex].push_back(intensity);
            }


            /******统计点云空间坐标最大值、最小值**********/
            xMax = (temp_x>xMax) ? temp_x : xMax;
            xMin = (temp_x<xMin) ? temp_x : xMin;
            yMax = (temp_y>yMax) ? temp_y : yMax;
            yMin = (temp_y<yMin) ? temp_y : yMin;
            zMax = (temp_z>zMax) ? temp_z : zMax;
            zMin = (temp_z<zMin) ? temp_z : zMin;

            //统计二维图像
            tofMax = (tof>tofMax) ? tof : tofMax;
            tofMin = (tof<tofMin) ? tof : tofMin;
            peakMax = (intensity>peakMax) ? intensity : peakMax;
            peakMin = (intensity<peakMin) ? intensity : peakMin;
        }
        else
            qDebug()<<QStringLiteral("给像素赋值时出现异常 imgrow=")<<imgRow<<"   imgCol = "<<imgCol<<endl;

    }  //一帧数据已经读取完成




    //统计信息相关的 ，将统计信息的容器赋值给全局变量
    if(statisticStartFlag)
    {
        statisticMutex.lock();
        allStatisticTofPoints = tempStatisticTofPoints;
        allStatisticPeakPoints = tempStatisticPeakPoints;
        statisticMutex.unlock();
    }



    // 1、将滤波功能放到这里进行实现，
    // 2、将滤波后的三维点云 同步到二维图像
    if(true == isFilterFlag)
    {

        /***********************接下来 根据点云的序号 去除二维图像中的噪声************************/

        mutex_3D.lock();
        tofImage = microQimage;
        intensityImage = macroQimage;
        pcl::copyPointCloud(tempRgbCloud_radius,pointCloudRgb);
        mutex_3D.unlock();
        /***************************************************************/

    }else{                      //不进行滤波
        mutex_3D.lock();
        tofImage = microQimage;
        intensityImage = macroQimage;
        pcl::copyPointCloud(tempRgbCloud,pointCloudRgb);
        mutex_3D.unlock();
    }
    isShowPointCloud = true;
}


/***********************************Raw Data部分 ***************************************************/
 QByteArray DealUsb_msg::StringToByte(QString str)
 {
     QByteArray byte_arr;
     bool ok;
     //如果str的长度 不是2的倍数  那么直接返回空
     //    if(str.size()%2!=0){
     //        qDebug()<<"is not twice "<<endl;
     //        return QByteArray::fromHex("字符串不符合标准");
     //    }

     str = str.replace(" ","");    //去掉空格键
     int len=str.size();
     for(int i=0;i<len;i+=2){
         byte_arr.append(char(str.mid(i,2).toUShort(&ok,16)));
     }
     //    qDebug()<<" byte_arr's len = "<<byte_arr.size()<<"    "<<byte_arr;

     //下面这段程序是将Byte 转化为QString的类型，将结果输出 对比发出的数据是否正确
     QDataStream out(&byte_arr,QIODevice::ReadWrite);
     QString strHex;
     while (!out.atEnd())
     {
         qint8 outChar=0;
         out>>outChar;
         QString str=QString("%1").arg(outChar&0xFF,2,16,QLatin1Char('0'));

         if (str.length()>1)
         {
             strHex+=str+" ";
         }
         else
         {
             strHex+="0"+str+" ";
         }
     }
     strHex = strHex.toUpper();
     qDebug()<<QStringLiteral("发送的原始数据为：")<<strHex<<"   len="<< strHex.length()/3<<endl;
     return byte_arr;
 }


//! \brief DealUsb_msg::UV910_RawData_deal_slot
//! \param array
//! 一共是1920000个字节 1920000*（4/5） = 1536000  == 640*2400
//! 800个字节对应于 640个点  ，对应于1次曝光的数据
//! 2400 对应于 20次积分
void DealUsb_msg::UV910_RawData_deal_slot(QByteArray array)
{

//    qDebug()<<"UV910_RawData_deal_slot(QByteArray array) size = "<<array.size();
//    QFile file("tmp.txt");
//    QString yf_position_line[200];
//    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
//    {
//        QTextStream in(&file);
//        int i = 0;
//        while (!in.atEnd())
//        {
//            yf_position_line[i] = in.readLine();
//            qDebug()<<"size = "<<yf_position_line[i].size();
//            i++;
//        }
//        file.close();
//    }

//    array = StringToByte(yf_position_line[0]);
//    qDebug()<<"array'size = "<<array.size();




    if(!isReceRawData_flag)
        return;
    if(currentRawDataFrame>rawData_saveFrameNums)
    {
        isReceRawData_flag = false;
        return;
    }

    int byteNum = array.size();
    int allExposureNum = byteNum / 800;
    QByteArray packageArray;   //一次曝光的数据
    float tmp1,tmp2,tmp3,tmp4;   //暂存的4个值
    int hongPixel_line = 0;   //宏像素所在的行  （0-29）
    int pixel_index = 0;      //每个宏像素中，微像素的标号：pixel0,pixle1,pixel2,pixel3; (0 1 2 3)
    int mic_line = 0;             //宏像素的第一行 还是第二行
    int mic_col = 0;              //宏像素的第一列 还是第二列
    int row_up = 0;           //上半部分像素所在的行
    int row_down =0;          //下班部分像素所在的行

    int col_1,col_2,col_3,col_4;
    int clouIndex1,clouIndex2,clouIndex3,clouIndex4;
    for(int i=0; i<allExposureNum; i++)  //曝光次数   （对于20次曝光，i的取值范围：0-2399）
    {
        hongPixel_line = i/(4*exposure_num);                 // 0.... 29
        pixel_index = i%(4*exposure_num)/exposure_num;       // 0 1 2 3
        mic_line = pixel_index/2;                            //宏像素的第一行 还是第二行
        mic_col = pixel_index%2;                            //宏像素的第一列 还是第二列
        row_up = 59 - ( 2*hongPixel_line + mic_line);      //上半部分像素所在的行
        row_down = 60 + ( 2*hongPixel_line + mic_line);    //下班部分像素所在的行

//        qDebug()<<"row_up = "<<row_up<<"  row_down="<<row_down;

        packageArray = array.mid(i*800,800);   //一行数据  800个字节，640个数据

        for(int j=0;j<160; j++ )               //四个数据为一组，一共有160组数据     (一个j对应4个数据)
        {
            tmp1 = ((quint8(packageArray[j*5+0])<<2) + ((quint8(packageArray[j*5+4])&0xC0)>>6));
            tmp2 = ((quint8(packageArray[j*5+1])<<2) + ((quint8(packageArray[j*5+4])&0x30)>>4));
            tmp3 = ((quint8(packageArray[j*5+2])<<2) + ((quint8(packageArray[j*5+4])&0x0C)>>2));
            tmp4 = ((quint8(packageArray[j*5+3])<<2) + ((quint8(packageArray[j*5+4])&0x03)));



            int packageNum = j/40;               //属于第几包数据(0 1 2 3)
            int tempIndex = j%40;                //0 1 2 3 ... 39    //每个包都归一化为 0-39的范围
            int upOrdownIndex = tempIndex%10;    //0-4:上半区  5-9：下半区    每个包的上半区 和下半区
            if(upOrdownIndex<5)
            {
                col_1 = mic_col + packageNum*2 + upOrdownIndex*(4*8);
                col_2 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8;
                col_3 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8*2;
                col_4 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8*3;
                clouIndex1 = row_up*160 + col_1;
                clouIndex2 = row_up*160 + col_2;
                clouIndex3 = row_up*160 + col_3;
                clouIndex4 = row_up*160 + col_4;

                rawDataSingle_str[clouIndex1].append(QString::number(tmp1)).append(" ");
                rawDataSingle_str[clouIndex2].append(QString::number(tmp2)).append(" ");
                rawDataSingle_str[clouIndex3].append(QString::number(tmp3)).append(" ");
                rawDataSingle_str[clouIndex4].append(QString::number(tmp4)).append(" ");

            }else
            {
                col_1 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8);
                col_2 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8;
                col_3 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8*2;
                col_4 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8*3;
                clouIndex1 = row_down*160 + col_1;
                clouIndex2 = row_down*160 + col_2;
                clouIndex3 = row_down*160 + col_3;
                clouIndex4 = row_down*160 + col_4;

                rawDataSingle_str[clouIndex1].append(QString::number(tmp1)).append(" ");
                rawDataSingle_str[clouIndex2].append(QString::number(tmp2)).append(" ");
                rawDataSingle_str[clouIndex3].append(QString::number(tmp3)).append(" ");
                rawDataSingle_str[clouIndex4].append(QString::number(tmp4)).append(" ");
            }
        }

    }  //一帧数据已经接收完毕
    currentRawDataFrame++;

    for(int k=0;k<19200;k++)
    {
        rawDataResult_str.append(rawDataSingle_str[k]).append('\n');
        rawDataSingle_str[k].clear();    //清空上本帧的数据
    }
    emit saveTXTSignal(rawDataResult_str);
    rawDataResult_str.clear();
}

//开始接收rowData数据
void DealUsb_msg::on_start_rawDataSave_slot(QString fiePath)
{
    currentRawDataFrame = 0;
    saveFilePath = fiePath;   //保存的路径  E:/..../.../的形式
    saveFileIndex = 1;      //文件标号；1作为开始
    qDebug()<<"rawData's savePath = "<<fiePath;
}



///***********************rowData MA 相关**************************************/
void DealUsb_msg::UV910_RawData_MA_deal_slot(QByteArray array)
{
    int byteNum = array.size();
    int allExposureNum = byteNum / 800;
    QByteArray packageArray;   //一次曝光的数据
    float tmp1,tmp2,tmp3,tmp4;   //暂存的4个值
    int hongPixel_line = 0;   //宏像素所在的行  （0-29）
    int pixel_index = 0;      //每个宏像素中，微像素的标号：pixel0,pixle1,pixel2,pixel3; (0 1 2 3)
    int mic_line = 0;             //宏像素的第一行 还是第二行
    int mic_col = 0;              //宏像素的第一列 还是第二列
    int row_up = 0;           //上半部分像素所在的行
    int row_down =0;          //下班部分像素所在的行

    int col_1,col_2,col_3,col_4;
    int clouIndex1,clouIndex2,clouIndex3,clouIndex4;
    for(int i=0; i<allExposureNum; i++)  //曝光次数   （对于20次曝光，i的取值范围：0-2399）
    {
        hongPixel_line = i/(4*exposure_num);                 // 0.... 29
        pixel_index = i%(4*exposure_num)/exposure_num;       // 0 1 2 3
        mic_line = pixel_index/2;                            //宏像素的第一行 还是第二行
        mic_col = pixel_index%2;                            //宏像素的第一列 还是第二列
        row_up = 59 - ( 2*hongPixel_line + mic_line);      //上半部分像素所在的行
        row_down = 60 + ( 2*hongPixel_line + mic_line);    //下班部分像素所在的行
        packageArray = array.mid(i*800,800);   //一行数据  800个字节，640个数据
        for(int j=0;j<160; j++ )               //四个数据为一组，一共有160组数据     (一个j对应4个数据)
        {
            tmp1 = ((quint8(packageArray[j*5+0])<<2) + ((quint8(packageArray[j*5+4])&0xC0)>>6));
            tmp2 = ((quint8(packageArray[j*5+1])<<2) + ((quint8(packageArray[j*5+4])&0x30)>>4));
            tmp3 = ((quint8(packageArray[j*5+2])<<2) + ((quint8(packageArray[j*5+4])&0x0C)>>2));
            tmp4 = ((quint8(packageArray[j*5+3])<<2) + ((quint8(packageArray[j*5+4])&0x03)));


            int packageNum = j/40;               //属于第几包数据(0 1 2 3)
            int tempIndex = j%40;                //0 1 2 3 ... 39    //每个包都归一化为 0-39的范围
            int upOrdownIndex = tempIndex%10;    //0-4:上半区  5-9：下半区    每个包的上半区 和下半区
            if(upOrdownIndex<5)
            {
                col_1 = mic_col + packageNum*2 + upOrdownIndex*(4*8);
                col_2 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8;
                col_3 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8*2;
                col_4 = mic_col + packageNum*2 + upOrdownIndex*(4*8) + 8*3;
                clouIndex1 = row_up*160 + col_1;
                clouIndex2 = row_up*160 + col_2;
                clouIndex3 = row_up*160 + col_3;
                clouIndex4 = row_up*160 + col_4;

                rawDataSingle_str[clouIndex1].append(QString::number(tmp1)).append(" ");
                rawDataSingle_str[clouIndex2].append(QString::number(tmp2)).append(" ");
                rawDataSingle_str[clouIndex3].append(QString::number(tmp3)).append(" ");
                rawDataSingle_str[clouIndex4].append(QString::number(tmp4)).append(" ");

            }else
            {
                col_1 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8);
                col_2 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8;
                col_3 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8*2;
                col_4 = mic_col + packageNum*2 +(upOrdownIndex - 5)*(4*8) + 8*3;
                clouIndex1 = row_down*160 + col_1;
                clouIndex2 = row_down*160 + col_2;
                clouIndex3 = row_down*160 + col_3;
                clouIndex4 = row_down*160 + col_4;

                rawDataSingle_str[clouIndex1].append(QString::number(tmp1)).append(" ");
                rawDataSingle_str[clouIndex2].append(QString::number(tmp2)).append(" ");
                rawDataSingle_str[clouIndex3].append(QString::number(tmp3)).append(" ");
                rawDataSingle_str[clouIndex4].append(QString::number(tmp4)).append(" ");
            }
        }

    }  //一帧数据已经接收完毕
    currentRawDataFrame++;

    for(int k=0;k<19200;k++)
    {
        rawDataMA_str.append(rawDataSingle_str[k]);
        rawDataSingle_str[k].clear();    //清空上本帧的数据
    }

    emit send_calMA_signal(rawDataMA_str);
    rawDataMA_str.clear();

}

