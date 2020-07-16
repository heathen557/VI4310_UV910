#include "calma_thread.h"

calMA_thread::calMA_thread(QObject *parent) : QObject(parent)
{
    qDebug()<<"calMA_thread(QObject *parent) start";

    //RawData MA 相关
    historgramVec_MA[0].resize(4096);
    historgramVec_MA[1].resize(4096);
    historgramVec_MA[2].resize(4096);
    historgramVec_MA[3].resize(4096);
    historgramVec_MA[4].resize(4096);
    histogram_maxValue = 0;

    currentFrame = 0;   //已经接收到的帧数
    histogram_frame = 0;   //用户设定的帧数

}

//!
//! \brief calMA_thread::start_RowData_bin_histogram_signal
//!接收rowData的要处理的信息
//! 开启binning rawData测试  曝光次数，积分次数，初始行、列、 窗口大小（2、4）、通道个数、 开启关闭标识
void calMA_thread::start_RowData_bin_histogram_slot(int exposureNum,int IntegrationNum,QVector<int> position,int winSize,int channelNum_,bool flag)
{
    qDebug()<<"exposureNum = "<<exposureNum;
    qDebug()<<"integrateNum = "<<IntegrationNum;
    histogram_frame = IntegrationNum/exposureNum;
    binning_winSize = winSize;
    channelNum = channelNum_;
    isSend_flag = flag;

    //需要binning 的像素点的位置
    if(1 == winSize)        //1x1窗口
    {
        //1通道
        histogram_binning_row[0][0] = position[0];
        histogram_binning_col[0][0] = position[1];
        //2通道
        histogram_binning_row[1][0] = position[2];
        histogram_binning_col[1][0] = position[3];
        //3通道
        histogram_binning_row[2][0] = position[4];
        histogram_binning_col[2][0] = position[5];
        //4通道
        histogram_binning_row[3][0] = position[6];
        histogram_binning_col[3][0] = position[7];

    }else if(2 == winSize)    //2x2的窗口
    {
        //1通道
        histogram_binning_row[0][0] = position[0];
        histogram_binning_row[0][1] = position[0]+1;
        histogram_binning_col[0][0] = position[1];
        histogram_binning_col[0][1] = position[1]+1;
        //2通道
        histogram_binning_row[1][0] = position[2];
        histogram_binning_row[1][1] = position[2]+1;
        histogram_binning_col[1][0] = position[3];
        histogram_binning_col[1][1] = position[3]+1;
        //3通道
        histogram_binning_row[2][0] = position[4];
        histogram_binning_row[2][1] = position[4]+1;
        histogram_binning_col[2][0] = position[5];
        histogram_binning_col[2][1] = position[5]+1;
        //4通道
        histogram_binning_row[3][0] = position[6];
        histogram_binning_row[3][1] = position[6]+1;
        histogram_binning_col[3][0] = position[7];
        histogram_binning_col[3][1] = position[7]+1;


    }else if(4 == winSize)   //4x4的窗口
    {
        //1通道
        histogram_binning_row[0][0] = position[0];
        histogram_binning_row[0][1] = position[0]+1;
        histogram_binning_row[0][2] = position[0]+2;
        histogram_binning_row[0][3] = position[0]+3;
        histogram_binning_col[0][0] = position[1];
        histogram_binning_col[0][1] = position[1]+1;
        histogram_binning_col[0][2] = position[1]+2;
        histogram_binning_col[0][3] = position[1]+3;
        //2通道
        histogram_binning_row[1][0] = position[2];
        histogram_binning_row[1][1] = position[2]+1;
        histogram_binning_row[1][2] = position[2]+2;
        histogram_binning_row[1][3] = position[2]+3;
        histogram_binning_col[1][0] = position[3];
        histogram_binning_col[1][1] = position[3]+1;
        histogram_binning_col[1][2] = position[3]+2;
        histogram_binning_col[1][3] = position[3]+3;
        //3通道
        histogram_binning_row[2][0] = position[4];
        histogram_binning_row[2][1] = position[4]+1;
        histogram_binning_row[2][2] = position[4]+2;
        histogram_binning_row[2][3] = position[4]+3;
        histogram_binning_col[2][0] = position[5];
        histogram_binning_col[2][1] = position[5]+1;
        histogram_binning_col[2][2] = position[5]+2;
        histogram_binning_col[2][3] = position[5]+3;
        //4通道
        histogram_binning_row[3][0] = position[6];
        histogram_binning_row[3][1] = position[6]+1;
        histogram_binning_row[3][2] = position[6]+2;
        histogram_binning_row[3][3] = position[6]+3;
        histogram_binning_col[3][0] = position[7];
        histogram_binning_col[3][1] = position[7]+1;
        histogram_binning_col[3][2] = position[7]+2;
        histogram_binning_col[3][3] = position[7]+3;
    }


}


//!
//! \brief calMA_thread::clearHistogram_slot
//!清空 所有的数据
void calMA_thread::clearHistogram_slot()
{
    for(int i=0; i<4096; i++)
    {
        historgramVec_MA[0][i] = 0;
        historgramVec_MA[1][i] = 0;
        historgramVec_MA[2][i] = 0;
        historgramVec_MA[3][i] = 0;
        historgramVec_MA[4][i] = 0;
    }
    currentFrame = 0;
}

//!
//! \brief calMA_thread::send_calMA_slot
//! \param rawDataMA_str  长度为19200 每个QString 包含 80个数据
//!接收完整一帧RawData的数据
void calMA_thread::send_calMA_slot(QStringList rawDataMA_str)
{
    currentFrame++;
    if(1 == binning_winSize)
    {
        //一通道
        int index_1 = histogram_binning_row[0][0] * 160 + histogram_binning_col[0][0];
        qDebug()<<"index_1 = "<<index_1;
        QString str = rawDataMA_str[index_1];
        QStringList strList = str.split(" ");

        for(int i=0; i<strList.size()-1; i++)
        {
            int hist = strList[i].toInt();
            historgramVec_MA[1][hist]++;
        }


        //二通道
        int index_2 = histogram_binning_row[1][0] * 160 + histogram_binning_col[1][0];
        str = rawDataMA_str[index_2];
        strList = str.split(" ");
        for(int i=0; i<strList.size()-1; i++ )
        {
            int hist  = strList[i].toInt();
            historgramVec_MA[2][hist]++;

        }

        //三通道
        int index_3 = histogram_binning_row[2][0] * 160 + histogram_binning_col[2][0];
        str = rawDataMA_str[index_3];
        strList = str.split(" ");
        for(int i=0; i<strList.size()-1; i++)
        {
            int hist = strList[i].toInt();
            historgramVec_MA[3][hist]++;
        }


        //四通道
        int index_4 = histogram_binning_row[3][0] * 160 + histogram_binning_col[3][0];
        str = rawDataMA_str[index_4];
        strList = str.split(" ");
        for(int i=0;i<strList.size()-1; i++)
        {
            int hist = strList[i].toInt();
            historgramVec_MA[4][hist]++;
        }

    }else if(2 == binning_winSize)   //binSize = 2    2x2
    {
        //一通道
        int index_11 = histogram_binning_row[0][0]*160 + histogram_binning_col[0][0];
        int index_12 = histogram_binning_row[0][0]*160 + histogram_binning_col[0][1];
        int index_13 = histogram_binning_row[0][1]*160 + histogram_binning_col[0][0];
        int index_14 = histogram_binning_row[0][1]*160 + histogram_binning_col[0][1];
        QString str1,str2,str3,str4;
        QStringList strList1,strList2,strList3,strList4;
        str1 = rawDataMA_str[index_11];
        str2 = rawDataMA_str[index_12];
        str3 = rawDataMA_str[index_13];
        str4 = rawDataMA_str[index_14];
        strList1 = str1.split(" ");
        strList2 = str2.split(" ");
        strList3 = str3.split(" ");
        strList4 = str4.split(" ");
        int hist1,hist2,hist3,hist4;
        for(int i=0;i<strList1.size()-1; i++)
        {
            hist1 = strList1[i].toInt();
            hist2 = strList2[i].toInt();
            hist3 = strList3[i].toInt();
            hist4 = strList4[i].toInt();

            historgramVec_MA[1][hist1]++;
            historgramVec_MA[1][hist2]++;
            historgramVec_MA[1][hist3]++;
            historgramVec_MA[1][hist4]++;
        }

        //二通道
        int index_21 = histogram_binning_row[1][0]* 160 + histogram_binning_col[1][0];
        int index_22 = histogram_binning_row[1][0]* 160 + histogram_binning_col[1][1];
        int index_23 = histogram_binning_row[1][1]* 160 + histogram_binning_col[1][0];
        int index_24 = histogram_binning_row[1][1]* 160 + histogram_binning_col[1][1];
        str1 = rawDataMA_str[index_21];
        str2 = rawDataMA_str[index_22];
        str3 = rawDataMA_str[index_23];
        str4 = rawDataMA_str[index_24];
        strList1 = str1.split(" ");
        strList2 = str2.split(" ");
        strList3 = str3.split(" ");
        strList4 = str4.split(" ");
        for(int i=0;i<strList1.size()-1; i++)
        {
            hist1 = strList1[i].toInt();
            hist2 = strList2[i].toInt();
            hist3 = strList3[i].toInt();
            hist4 = strList4[i].toInt();

            historgramVec_MA[2][hist1]++;
            historgramVec_MA[2][hist2]++;
            historgramVec_MA[2][hist3]++;
            historgramVec_MA[2][hist4]++;
        }


        //三通道
        int index_31 = histogram_binning_row[2][0] * 160 + histogram_binning_col[2][0];
        int index_32 = histogram_binning_row[2][0] * 160 + histogram_binning_col[2][1];
        int index_33 = histogram_binning_row[2][1] * 160 + histogram_binning_col[2][0];
        int index_34 = histogram_binning_row[2][1] * 160 + histogram_binning_col[2][1];
        str1 = rawDataMA_str[index_31];
        str2 = rawDataMA_str[index_32];
        str3 = rawDataMA_str[index_33];
        str4 = rawDataMA_str[index_34];
        strList1 = str1.split(" ");
        strList2 = str2.split(" ");
        strList3 = str3.split(" ");
        strList4 = str4.split(" ");
        for(int i=0;i<strList1.size()-1; i++)
        {
            hist1 = strList1[i].toInt();
            hist2 = strList2[i].toInt();
            hist3 = strList3[i].toInt();
            hist4 = strList4[i].toInt();

            historgramVec_MA[3][hist1]++;
            historgramVec_MA[3][hist2]++;
            historgramVec_MA[3][hist3]++;
            historgramVec_MA[3][hist4]++;
        }

        //四通道
        int index_41 = histogram_binning_row[3][0] * 160 + histogram_binning_col[3][0];
        int index_42 = histogram_binning_row[3][0] * 160 + histogram_binning_col[3][1];
        int index_43 = histogram_binning_row[3][1] * 160 + histogram_binning_col[3][0];
        int index_44 = histogram_binning_row[3][1] * 160 + histogram_binning_col[3][1];
        str1 = rawDataMA_str[index_41];
        str2 = rawDataMA_str[index_42];
        str3 = rawDataMA_str[index_43];
        str4 = rawDataMA_str[index_44];
        strList1 = str1.split(" ");
        strList2 = str2.split(" ");
        strList3 = str3.split(" ");
        strList4 = str4.split(" ");
        for(int i=0;i<strList1.size()-1; i++)
        {
            hist1 = strList1[i].toInt();
            hist2 = strList2[i].toInt();
            hist3 = strList3[i].toInt();
            hist4 = strList4[i].toInt();

            historgramVec_MA[4][hist1]++;
            historgramVec_MA[4][hist2]++;
            historgramVec_MA[4][hist3]++;
            historgramVec_MA[4][hist4]++;
        }


    }else if(4 == binning_winSize)   // 4x4 的Bin
    {
        //一通道
        int index_11 = histogram_binning_row[0][0] *160 + histogram_binning_col[0][0];
        int index_12 = histogram_binning_row[0][0] *160 + histogram_binning_col[0][1];
        int index_13 = histogram_binning_row[0][0] *160 + histogram_binning_col[0][2];
        int index_14 = histogram_binning_row[0][0] *160 + histogram_binning_col[0][3];
        int index_15 = histogram_binning_row[0][1] *160 + histogram_binning_col[0][0];
        int index_16 = histogram_binning_row[0][1] *160 + histogram_binning_col[0][1];
        int index_17 = histogram_binning_row[0][1] *160 + histogram_binning_col[0][2];
        int index_18 = histogram_binning_row[0][1] *160 + histogram_binning_col[0][3];
        int index_19 = histogram_binning_row[0][2] *160 + histogram_binning_col[0][0];
        int index_1A = histogram_binning_row[0][2] *160 + histogram_binning_col[0][1];
        int index_1B = histogram_binning_row[0][2] *160 + histogram_binning_col[0][2];
        int index_1C = histogram_binning_row[0][2] *160 + histogram_binning_col[0][3];
        int index_1D = histogram_binning_row[0][3] *160 + histogram_binning_col[0][0];
        int index_1E = histogram_binning_row[0][3] *160 + histogram_binning_col[0][1];
        int index_1F = histogram_binning_row[0][3] *160 + histogram_binning_col[0][2];
        int index_1G = histogram_binning_row[0][3] *160 + histogram_binning_col[0][3];
        QString str[16];
        QStringList strList[16];
        str[0] = rawDataMA_str[index_11];
        str[1] = rawDataMA_str[index_12];
        str[2] = rawDataMA_str[index_13];
        str[3] = rawDataMA_str[index_14];
        str[4] = rawDataMA_str[index_15];
        str[5] = rawDataMA_str[index_16];
        str[6] = rawDataMA_str[index_17];
        str[7] = rawDataMA_str[index_18];
        str[8] = rawDataMA_str[index_19];
        str[9] = rawDataMA_str[index_1A];
        str[10] = rawDataMA_str[index_1B];
        str[11] = rawDataMA_str[index_1C];
        str[12] = rawDataMA_str[index_1D];
        str[13] = rawDataMA_str[index_1E];
        str[14] = rawDataMA_str[index_1F];
        str[15] = rawDataMA_str[index_1G];
        for(int i=0; i<16; i++)
        {
            strList[i] = str[i].split(" ");
        }
        int hist;
        for(int i=0; i<16; i++)
        {
            for(int j=0; j<strList[0].size()-1; j++)
            {
                hist = strList[i][j].toInt();
                historgramVec_MA[1][hist]++;
            }
        }




        //二通道
        int index_21 = histogram_binning_row[1][0] *160 + histogram_binning_col[1][0];
        int index_22 = histogram_binning_row[1][0] *160 + histogram_binning_col[1][1];
        int index_23 = histogram_binning_row[1][0] *160 + histogram_binning_col[1][2];
        int index_24 = histogram_binning_row[1][0] *160 + histogram_binning_col[1][3];
        int index_25 = histogram_binning_row[1][1] *160 + histogram_binning_col[1][0];
        int index_26 = histogram_binning_row[1][1] *160 + histogram_binning_col[1][1];
        int index_27 = histogram_binning_row[1][1] *160 + histogram_binning_col[1][2];
        int index_28 = histogram_binning_row[1][1] *160 + histogram_binning_col[1][3];
        int index_29 = histogram_binning_row[1][2] *160 + histogram_binning_col[1][0];
        int index_2A = histogram_binning_row[1][2] *160 + histogram_binning_col[1][1];
        int index_2B = histogram_binning_row[1][2] *160 + histogram_binning_col[1][2];
        int index_2C = histogram_binning_row[1][2] *160 + histogram_binning_col[1][3];
        int index_2D = histogram_binning_row[1][3] *160 + histogram_binning_col[1][0];
        int index_2E = histogram_binning_row[1][3] *160 + histogram_binning_col[1][1];
        int index_2F = histogram_binning_row[1][3] *160 + histogram_binning_col[1][2];
        int index_2G = histogram_binning_row[1][3] *160 + histogram_binning_col[1][3];

        str[0] = rawDataMA_str[index_21];
        str[1] = rawDataMA_str[index_22];
        str[2] = rawDataMA_str[index_23];
        str[3] = rawDataMA_str[index_24];
        str[4] = rawDataMA_str[index_25];
        str[5] = rawDataMA_str[index_26];
        str[6] = rawDataMA_str[index_27];
        str[7] = rawDataMA_str[index_28];
        str[8] = rawDataMA_str[index_29];
        str[9] = rawDataMA_str[index_2A];
        str[10] = rawDataMA_str[index_2B];
        str[11] = rawDataMA_str[index_2C];
        str[12] = rawDataMA_str[index_2D];
        str[13] = rawDataMA_str[index_2E];
        str[14] = rawDataMA_str[index_2F];
        str[15] = rawDataMA_str[index_2G];
        for(int i=0; i<16; i++)
        {
            strList[i] = str[i].split(" ");
        }

        for(int i=0; i<16; i++)
        {
            for(int j=0; j<strList[0].size()-1; j++)
            {
                hist = strList[i][j].toInt();
                historgramVec_MA[2][hist]++;
            }
        }


        //三通道
        int index_31 = histogram_binning_row[2][0] *160 + histogram_binning_col[2][0];
        int index_32 = histogram_binning_row[2][0] *160 + histogram_binning_col[2][1];
        int index_33 = histogram_binning_row[2][0] *160 + histogram_binning_col[2][2];
        int index_34 = histogram_binning_row[2][0] *160 + histogram_binning_col[2][3];
        int index_35 = histogram_binning_row[2][1] *160 + histogram_binning_col[2][0];
        int index_36 = histogram_binning_row[2][1] *160 + histogram_binning_col[2][1];
        int index_37 = histogram_binning_row[2][1] *160 + histogram_binning_col[2][2];
        int index_38 = histogram_binning_row[2][1] *160 + histogram_binning_col[2][3];
        int index_39 = histogram_binning_row[2][2] *160 + histogram_binning_col[2][0];
        int index_3A = histogram_binning_row[2][2] *160 + histogram_binning_col[2][1];
        int index_3B = histogram_binning_row[2][2] *160 + histogram_binning_col[2][2];
        int index_3C = histogram_binning_row[2][2] *160 + histogram_binning_col[2][3];
        int index_3D = histogram_binning_row[2][3] *160 + histogram_binning_col[2][0];
        int index_3E = histogram_binning_row[2][3] *160 + histogram_binning_col[2][1];
        int index_3F = histogram_binning_row[2][3] *160 + histogram_binning_col[2][2];
        int index_3G = histogram_binning_row[2][3] *160 + histogram_binning_col[2][3];

        str[0] = rawDataMA_str[index_31];
        str[1] = rawDataMA_str[index_32];
        str[2] = rawDataMA_str[index_33];
        str[3] = rawDataMA_str[index_34];
        str[4] = rawDataMA_str[index_35];
        str[5] = rawDataMA_str[index_36];
        str[6] = rawDataMA_str[index_37];
        str[7] = rawDataMA_str[index_38];
        str[8] = rawDataMA_str[index_39];
        str[9] = rawDataMA_str[index_3A];
        str[10] = rawDataMA_str[index_3B];
        str[11] = rawDataMA_str[index_3C];
        str[12] = rawDataMA_str[index_3D];
        str[13] = rawDataMA_str[index_3E];
        str[14] = rawDataMA_str[index_3F];
        str[15] = rawDataMA_str[index_3G];
        for(int i=0; i<16; i++)
        {
            strList[i] = str[i].split(" ");
        }

        for(int i=0; i<16; i++)
        {
            for(int j=0; j<strList[0].size()-1; j++)
            {
                hist = strList[i][j].toInt();
                historgramVec_MA[3][hist]++;
            }
        }



        //四通道
        int index_41 = histogram_binning_row[3][0] *160 + histogram_binning_col[3][0];
        int index_42 = histogram_binning_row[3][0] *160 + histogram_binning_col[3][1];
        int index_43 = histogram_binning_row[3][0] *160 + histogram_binning_col[3][2];
        int index_44 = histogram_binning_row[3][0] *160 + histogram_binning_col[3][3];
        int index_45 = histogram_binning_row[3][1] *160 + histogram_binning_col[3][0];
        int index_46 = histogram_binning_row[3][1] *160 + histogram_binning_col[3][1];
        int index_47 = histogram_binning_row[3][1] *160 + histogram_binning_col[3][2];
        int index_48 = histogram_binning_row[3][1] *160 + histogram_binning_col[3][3];
        int index_49 = histogram_binning_row[3][2] *160 + histogram_binning_col[3][0];
        int index_4A = histogram_binning_row[3][2] *160 + histogram_binning_col[3][1];
        int index_4B = histogram_binning_row[3][2] *160 + histogram_binning_col[3][2];
        int index_4C = histogram_binning_row[3][2] *160 + histogram_binning_col[3][3];
        int index_4D = histogram_binning_row[3][3] *160 + histogram_binning_col[3][0];
        int index_4E = histogram_binning_row[3][3] *160 + histogram_binning_col[3][1];
        int index_4F = histogram_binning_row[3][3] *160 + histogram_binning_col[3][2];
        int index_4G = histogram_binning_row[3][3] *160 + histogram_binning_col[3][3];

        str[0] = rawDataMA_str[index_41];
        str[1] = rawDataMA_str[index_42];
        str[2] = rawDataMA_str[index_43];
        str[3] = rawDataMA_str[index_44];
        str[4] = rawDataMA_str[index_45];
        str[5] = rawDataMA_str[index_46];
        str[6] = rawDataMA_str[index_47];
        str[7] = rawDataMA_str[index_48];
        str[8] = rawDataMA_str[index_49];
        str[9] = rawDataMA_str[index_4A];
        str[10] = rawDataMA_str[index_4B];
        str[11] = rawDataMA_str[index_4C];
        str[12] = rawDataMA_str[index_4D];
        str[13] = rawDataMA_str[index_4E];
        str[14] = rawDataMA_str[index_4F];
        str[15] = rawDataMA_str[index_4G];
        for(int i=0; i<16; i++)
        {
            strList[i] = str[i].split(" ");
        }
        for(int i=0; i<16; i++)
        {
            for(int j=0; j<strList[0].size()-1; j++)
            {
                hist = strList[i][j].toInt();
                historgramVec_MA[4][hist]++;
            }
        }
    }




    if(isSend_flag)
    {
        emit toShowHistogram_channel1_signal(historgramVec_MA[1],0);   //实时发送数据
        emit toShowHistogram_channel2_signal(historgramVec_MA[2],0);
        emit toShowHistogram_channel3_signal(historgramVec_MA[3],0);
        emit toShowHistogram_channel4_signal(historgramVec_MA[4],0);

        emit sendFrameIndex_signal(currentFrame);

    }




    if(currentFrame>=histogram_frame) //发送一次直方图数据
    {
//        emit toShowHistogram_channel1_signal(historgramVec_MA[1],0);
//        emit toShowHistogram_channel2_signal(historgramVec_MA[2],0);
//        emit toShowHistogram_channel3_signal(historgramVec_MA[3],0);
//        emit toShowHistogram_channel4_signal(historgramVec_MA[4],0);

        for(int i=0;i<historgramVec_MA[1].size(); i++)
        {
            historgramVec_MA[1][i] = 0;
            historgramVec_MA[2][i] = 0;
            historgramVec_MA[3][i] = 0;
            historgramVec_MA[4][i] = 0;
        }
        currentFrame = 0;
    }



}


