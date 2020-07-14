﻿#ifndef HIST_MA_DIALOG_H
#define HIST_MA_DIALOG_H

#include <QDialog>
#include"qcustomplot.h"
#include"filterwindow_dialog.h"


// alter 2020-07-14
// add: TDC_min : TDC_max
// add: show holdValue ratio
// add: saveHistogram
// reStart: clear


namespace Ui {
class Hist_MA_Dialog;
}

class Hist_MA_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Hist_MA_Dialog(QWidget *parent = 0);
    ~Hist_MA_Dialog();

    void enableUI_trueOrfalse(bool flag);

    void init_UI();
    void init_histogramLabel();

    int selectRow;
    int selectCol;

    QVector<double> filterWindow;

    QCPBars *regen[4];          //直方图用   *****标签相关
    void init_histogram();      //初始化直方图数据

    QVector<double> HistorgramTicks[4];      //标签
    QVector<QString> HistorgramLabels[4];    //标识
    int binSize;

    float summary;


    int channelNum;   //通道个数

    int TDC_min;
    int TDC_max;




public slots:
    void on_channel_1_row_comboBox_activated(int index);

    void on_channel_1_col_comboBox_activated(int index);

    void on_start_pushButton_clicked();

//    void toShowHistogram_slot(QVector<double> histogram_vec,int maxValue);

    void on_filters_pushButton_clicked();

    void send_filterWindow_slot(QStringList);

    void currentFrame_slot(int frame);   //看当前已经曝光的多少次数

    void on_binning_size_comboBox_activated(int index);

    void on_bin_start_pushButton_clicked();

    void on_binSize_comboBox_activated(const QString &arg1);

    void on_channels_comboBox_currentIndexChanged(const QString &arg1);


    void toShowHistogram_channel1_slot(QVector<double> histogram_vec,int maxValue);
    void toShowHistogram_channel2_slot(QVector<double> histogram_vec,int maxValue);
    void toShowHistogram_channel3_slot(QVector<double> histogram_vec,int maxValue);
    void toShowHistogram_channel4_slot(QVector<double> histogram_vec,int maxValue);


    void sendFrameIndex_slot(int );
signals:

    //告知UV910接收线程  开启接收rowData MA 的模式
    void startReceUV910_rowData_MA_signal(bool );

//    void start_RowDatahistogram_signal(int ,int, int row,int col, bool);              //单个pixel  曝光次数，积分次数 ，行、列、 开启关闭标识
    void start_RowData_bin_histogram_signal(int,int,QVector<int>,int,int,bool);          //开启binning rawData测试  曝光次数，积分次数，初始行、列、 窗口大小（2、4）、通道个数、 开启关闭标识

    void clearHistogram_signal();

private slots:
    void on_TDC_min_lineEdit_returnPressed();

    void on_TDC_max_lineEdit_returnPressed();

    void on_save_MA1_pushButton_clicked();

    void on_save_MA2_pushButton_clicked();

    void on_save_MA3_pushButton_clicked();

    void on_save_MA4_pushButton_clicked();

private:
    Ui::Hist_MA_Dialog *ui;

    filterWindow_Dialog filterWin_dia;
};

#endif // HIST_MA_DIALOG_H
