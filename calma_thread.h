#ifndef CALMA_THREAD_H
#define CALMA_THREAD_H

#include <QObject>
#include<QDebug>

class calMA_thread : public QObject
{
    Q_OBJECT
public:
    explicit calMA_thread(QObject *parent = 0);


    QVector<double> historgramVec_MA[5];  //存储直方图的数据   支持最大4096 ；  4：代表最大支持4个通道
    int RawData_model ;                   // 1、单个pixel支持多通道的模式   2、binning 模式   3、CD_model 模式   4、stuck_cast模式
    int channelNum;                       //通道个数
    int histogram_binning_row[4][4];      //因为需要支持4通道 ， 4X4的 binning  row
    int histogram_binning_col[4][4];      //因为需要支持4通道 ， 4x4的 binning  col
    int histogram_frame;                 //多少帧出一次直方图数据
    int histogram_maxValue;
    bool isRecvRowDataMA_flag;
    int binning_winSize;


    int currentFrame;

    bool isSend_flag;
signals:

    void toShowHistogram_channel1_signal(QVector<double>,int);
    void toShowHistogram_channel2_signal(QVector<double>,int);
    void toShowHistogram_channel3_signal(QVector<double>,int);
    void toShowHistogram_channel4_signal(QVector<double>,int);

    void sendFrameIndex_signal(int);

public slots:

    //接收一帧rowData数据
    void send_calMA_slot(QStringList rawDataMA_str);

    //接收要处理的rowdata
    //开启binning rawData测试  曝光次数，积分次数，初始行、列、 窗口大小（2、4）、通道个数、 开启关闭标识
    void start_RowData_bin_histogram_slot(int,int,QVector<int>,int,int,bool);


    //数据清空
    void clearHistogram_slot();
};

#endif // CALMA_THREAD_H
