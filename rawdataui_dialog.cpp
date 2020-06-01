﻿#include "rawdataui_dialog.h"
#include "ui_rawdataui_dialog.h"

extern QString rawData_savePath;    //保存的路径
extern int exposure_num;            //曝光次数
extern int rawData_saveFrameNums;   //保存帧数
extern bool isReceRawData_flag;     //是否保存rawData


rawDataUI_Dialog::rawDataUI_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rawDataUI_Dialog)
{
    ui->setupUi(this);
}

rawDataUI_Dialog::~rawDataUI_Dialog()
{
    delete ui;
}

//!选择存储路径
void rawDataUI_Dialog::on_SelFilePath_pushButton_clicked()
{
    file_path = QFileDialog::getExistingDirectory(this,QStringLiteral("请选择文件保存路径..."),"./");
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
        ui->lineEdit->setText(file_path);
    }
}

//!
//! \brief rawDataUI_Dialog::on_start_pushButton_clicked
//!开始接收RawData数据，并开始保存
void rawDataUI_Dialog::on_start_pushButton_clicked()
{
    if(isReceRawData_flag)
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("已经处于rawData数据的保存模式下"));
        return;
    }


    exposure_num = ui->exposure_lineEdit->text().toInt();
    rawData_saveFrameNums = ui->saveFrameNum_lineEdit->text().toInt();
    rawData_savePath = ui->lineEdit->text();
    isReceRawData_flag = true;
    qDebug()<<"on_start_pushButton_clicked()   exposure_num"<<exposure_num<<"    rawPath ="<<rawData_savePath;

    emit startReceUV910_rowData_signal(true);
    emit on_start_rawDataSave_signal(rawData_savePath);

//    this->hide();
}

//!
//! \brief rawDataUI_Dialog::send_savedFileIndex_slot
//!显示已经存储了多少帧
void rawDataUI_Dialog::send_savedFileIndex_slot(int frame)
{
    ui->frame_label->setText(QString::number(frame-1));
}
