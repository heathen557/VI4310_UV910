#include"globaldata.h"

using namespace std;

/*保存用到的标识*/
bool isSaveFlag;        //是否进行存储
QString saveFilePath;   //保存的路径  E:/..../.../的形式
int saveFileIndex;      //文件标号；1作为开始

/*******统计信息相关的变量***********/
QMutex statisticMutex;
vector<vector<int>> allStatisticTofPoints;   //用于统计 均值和方差的 容器  TOF
vector<vector<int>> allStatisticPeakPoints;   //用于统计 均值和方差的 容器  TOF

//3D 2D图像显示相关
QMutex mutex_3D;  //3D点云/2D传输的互斥锁
QImage tofImage;
QImage intensityImage;
pcl::PointCloud<pcl::PointXYZRGB> pointCloudRgb;
bool isShowPointCloud;  //是否有点云数据 ，有的话显示否则不显示

//鼠标点击显示时相关
QMutex mouseShowMutex;
float mouseShowTOF[160][120];
float mouseShowPEAK[160][120];
float mouseShowDepth[160][120];
float mouseShow_X[160][120];
float mouseShow_Z[160][120];


//数据接收相关
bool isRecvFlag;

//UV910数据接收相关
bool isReceUV910_flag;  //是否接收数据的标识
SensorTab CurrentSensor;
int m_RunMode;
int m_nDevID;


//rawData 数据保存相关
QString rawData_savePath;    //保存的路径
int rawData_saveFrameNums;   //保存的帧数
int exposure_num;            //曝光次数
